#include "object_file/pdb.h"

#include <llvm/DebugInfo/CodeView/SymbolDeserializer.h>
#include <llvm/DebugInfo/CodeView/TypeStreamMerger.h>
#include <llvm/DebugInfo/MSF/MSFBuilder.h>
#include <llvm/DebugInfo/PDB/IPDBSession.h>
#include <llvm/DebugInfo/PDB/Native/DbiStreamBuilder.h>
#include <llvm/DebugInfo/PDB/Native/GSIStreamBuilder.h>
#include <llvm/DebugInfo/PDB/Native/InfoStreamBuilder.h>
#include <llvm/DebugInfo/PDB/Native/PDBFile.h>
#include <llvm/DebugInfo/PDB/Native/PublicsStream.h>
#include <llvm/DebugInfo/PDB/Native/RawConstants.h>
#include <llvm/DebugInfo/PDB/Native/SymbolStream.h>
#include <llvm/DebugInfo/PDB/Native/TpiStream.h>
#include <llvm/DebugInfo/PDB/Native/TpiStreamBuilder.h>
#include <llvm/DebugInfo/PDB/PDB.h>
#include <llvm/DebugInfo/PDB/PDBTypes.h>

#include "util/llvm_error.h"

using namespace llvm::pdb;
using namespace llvm::codeview;

namespace di::object_file {

void PDB::read(const fs::path& path) {
    check_llvm_result(
        loadDataForPDB(PDB_ReaderType::Native, path.string(), m_session)
    );

    m_storaged_Ipi.reset(new codeview::MergingTypeTableBuilder(m_Alloc));
    m_storaged_Tpi.reset(new codeview::MergingTypeTableBuilder(m_Alloc));

    auto& pdb_file = get_native_session().getPDBFile();

    SmallVector<codeview::TypeIndex, 128> type_map;
    SmallVector<codeview::TypeIndex, 128> id_map;

    auto& tpi_stream = check_llvm_result(pdb_file.getPDBTpiStream());

    check_llvm_result(codeview::mergeTypeRecords(
        *m_storaged_Tpi,
        type_map,
        tpi_stream.typeArray()
    ));

    auto& ipi_stream = check_llvm_result(pdb_file.getPDBIpiStream());

    check_llvm_result(codeview::mergeIdRecords(
        *m_storaged_Ipi,
        type_map,
        id_map,
        ipi_stream.typeArray()
    ));
}

void PDB::_write(const fs::path& path) {
    build();

    GUID out_guid;
    check_llvm_result(m_builder->commit(path.string(), &out_guid));
}

NativeSession& PDB::get_native_session() {
    return *static_cast<NativeSession*>(m_session.get());
}

pdb::PDBFile& PDB::get_pdb_file() { return get_native_session().getPDBFile(); }

void PDB::_for_each_public(const for_each_symbol_callback_t& callback) {
    using namespace codeview;
    auto& file           = get_pdb_file();
    auto& publics_stream = check_llvm_result(file.getPDBPublicsStream());
    auto& symbol_stream  = check_llvm_result(file.getPDBSymbolStream());

    auto raw_stream = symbol_stream.getSymbolArray().getUnderlyingStream();
    for (auto offset : publics_stream.getPublicsTable()) {
        auto cv_symbol    = readSymbolFromStream(raw_stream, offset);
        auto public_sym32 = check_llvm_result(
            SymbolDeserializer::deserializeAs<PublicSym32>(cv_symbol.get()),
            "Unsupported symbol type."
        );
        callback(public_sym32);
    }
}

void PDB::build() {
    m_builder.reset(new PDBFileBuilder{m_Alloc});

    constexpr auto block_size = 4096;
    check_llvm_result(m_builder->initialize(block_size));

    for (uint32_t idx = 0; idx < pdb::kSpecialStreamCount; ++idx) {
        check_llvm_result(m_builder->getMsfBuilder().addStream(0));
    }

    // INFO
    {
        auto  pdb_info = m_owning_coff->get_debug_info();
        auto& Info     = m_builder->getInfoBuilder();

        Info.setVersion(PdbRaw_ImplVer::PdbImplVC70);
        Info.setAge(pdb_info.Age);
        Info.setGuid(*reinterpret_cast<GUID*>(pdb_info.Signature));
        Info.addFeature(PdbRaw_FeatureSig::VC140);
    }

    // DBI
    {
        auto  pdb_info = m_owning_coff->get_debug_info();
        auto& Dbi      = m_builder->getDbiBuilder();

        Dbi.setVersionHeader(PdbRaw_DbiVer::PdbDbiV70);
        Dbi.setAge(pdb_info.Age);
        Dbi.setMachineType(PDB_Machine::Amd64);
        Dbi.setFlags(DbiFlags::FlagStrippedMask);
        Dbi.setBuildNumber(14, 11); // LLVM is compatible with LINK 14.11

        // Add sections.
        auto section_table      = m_owning_coff->get_section_table();
        auto number_of_sections = m_owning_coff->get_number_of_sections();

        auto section_data_ref = ArrayRef<uint8_t>(
            (uint8_t*)section_table,
            m_owning_coff->get_number_of_sections()
                * sizeof(object::coff_section)
        );

        auto section_table_ref = ArrayRef<object::coff_section>(
            (const object::coff_section*)section_data_ref.data(),
            number_of_sections
        );

        Dbi.createSectionMap(section_table_ref);

        // Add COFF section header stream.
        check_llvm_result(
            Dbi.addDbgStream(DbgHeaderType::SectionHdr, section_data_ref)
        );
    }

    // TPI & IPI
    {
        auto& Tpi = m_builder->getTpiBuilder();
        auto& Ipi = m_builder->getIpiBuilder();

        Tpi.setVersionHeader(PdbRaw_TpiVer::PdbTpiV80);
        Ipi.setVersionHeader(PdbRaw_TpiVer::PdbTpiV80);

        if (m_storaged_Tpi) {
            for_each<TPI>([&Tpi](TypeIndex index, const CVType& type) {
                Tpi.addTypeRecord(type.RecordData, std::nullopt);
            });
        }

        if (m_storaged_Ipi) {
            for_each<IPI>([&Ipi](TypeIndex index, const CVType& type) {
                Ipi.addTypeRecord(type.RecordData, std::nullopt);
            });
        }
    }

    // PUBLIC
    {
        std::vector<BulkPublic> publics;
        m_owning_symbol_data->for_each([&publics,
                                        this](const BoundSymbol& entity) {
            BulkPublic symbol;

            auto section_index = m_owning_coff->get_section_index(entity.m_rva);
            auto section       = check_llvm_result(
                m_owning_coff->get_owning_coff().getSection(section_index + 1)
            );

            symbol.Name    = strdup(entity.m_symbol_name.c_str());
            symbol.NameLen = entity.m_symbol_name.size();
            symbol.Segment = section_index + 1;
            symbol.Offset  = entity.m_rva - section->VirtualAddress;
            if (entity.m_is_function) symbol.setFlags(PublicSymFlags::Function);

            publics.emplace_back(symbol);
        });

        m_builder->getGsiBuilder().addPublicSymbols(std::move(publics));
    }
}

} // namespace di::object_file
