#include "object_file/PDB.h"

#include <llvm/DebugInfo/MSF/MSFBuilder.h>
#include <llvm/DebugInfo/PDB/Native/DbiStreamBuilder.h>
#include <llvm/DebugInfo/PDB/Native/GSIStreamBuilder.h>
#include <llvm/DebugInfo/PDB/Native/InfoStreamBuilder.h>
#include <llvm/DebugInfo/PDB/Native/RawConstants.h>
#include <llvm/DebugInfo/PDB/Native/TpiStreamBuilder.h>

using namespace llvm::pdb;

namespace di::object_file {

PDB::PDB() : m_builder(m_allocator) {
    constexpr uint32_t block_size = 4096;
    if (m_builder.initialize(block_size)) {
        throw std::runtime_error("Failed to initialize pdb file builder.");
    }

    for (uint32_t I = 0; I < pdb::kSpecialStreamCount; ++I) {
        if (!m_builder.getMsfBuilder().addStream(0)) {
            throw std::runtime_error("Failed to add initial stream.");
        }
    }
}

void PDB::write(const std::filesystem::path& path) {
    build();

    codeview::GUID out_guid;
    if (m_builder.commit(path.string(), &out_guid)) {
        throw std::runtime_error("Failed to create pdb!");
    }
}

void PDB::build() {
    build_Info();
    build_DBI();
    build_TPI();
    build_GSI();
}

void PDB::build_Info() {
    auto  pdb_info = m_owning_coff->get_debug_info();
    auto& Info     = m_builder.getInfoBuilder();

    Info.setVersion(PdbRaw_ImplVer::PdbImplVC70);
    Info.setAge(pdb_info.Age);
    Info.setGuid(*reinterpret_cast<codeview::GUID*>(pdb_info.Signature));
    Info.addFeature(PdbRaw_FeatureSig::VC140);
}

void PDB::build_DBI() {
    auto  pdb_info = m_owning_coff->get_debug_info();
    auto& DBI      = m_builder.getDbiBuilder();

    DBI.setVersionHeader(PdbRaw_DbiVer::PdbDbiV70);
    DBI.setAge(pdb_info.Age);
    DBI.setMachineType(PDB_Machine::Amd64);
    DBI.setFlags(DbiFlags::FlagStrippedMask);
    DBI.setBuildNumber(14, 11); // LLVM is compatible with LINK 14.11

    // Add sections.
    auto section_table      = m_owning_coff->get_section_table();
    auto number_of_sections = m_owning_coff->get_number_of_sections();

    auto section_data_ref = ArrayRef<uint8_t>(
        (uint8_t*)section_table,
        m_owning_coff->get_number_of_sections() * sizeof(object::coff_section)
    );

    auto section_table_ref = ArrayRef<object::coff_section>(
        (const object::coff_section*)section_data_ref.data(),
        number_of_sections
    );

    DBI.createSectionMap(section_table_ref);

    // Add COFF section header stream.
    if (DBI.addDbgStream(DbgHeaderType::SectionHdr, section_data_ref)) {
        throw std::runtime_error("Failed to add dbg stream.");
    }
}

void PDB::build_TPI() {
    auto& TPI = m_builder.getTpiBuilder();
    auto& IPI = m_builder.getIpiBuilder();

    TPI.setVersionHeader(PdbRaw_TpiVer::PdbTpiV80);
    IPI.setVersionHeader(PdbRaw_TpiVer::PdbTpiV80);

    if (m_owning_raw_type_data) {
        m_owning_raw_type_data->for_each<data_format::RawTypeData::TPI>(
            [&TPI](codeview::TypeIndex index, const codeview::CVType& type) {
                TPI.addTypeRecord(type.RecordData, std::nullopt);
            }
        );
        m_owning_raw_type_data->for_each<data_format::RawTypeData::IPI>(
            [&IPI](codeview::TypeIndex index, const codeview::CVType& type) {
                IPI.addTypeRecord(type.RecordData, std::nullopt);
            }
        );
    }
}

void PDB::build_GSI() {
    std::vector<BulkPublic> publics;
    m_owning_symbol_data->for_each([&publics, this](const BoundSymbol& entity) {
        BulkPublic symbol;

        auto section_index =
            m_owning_coff->get_section_index(entity.m_rva - m_image_base);
        auto section_or_err =
            m_owning_coff->get_owning_coff().getSection(section_index + 1);
        if (!section_or_err) {
            throw std::runtime_error("Invalid section.");
        }

        symbol.Name    = strdup(entity.m_symbol_name.c_str());
        symbol.NameLen = entity.m_symbol_name.size();
        symbol.Segment = section_index + 1;
        symbol.Offset =
            entity.m_rva - m_image_base - section_or_err.get()->VirtualAddress;
        if (entity.m_is_function)
            symbol.setFlags(codeview::PublicSymFlags::Function);

        publics.emplace_back(symbol);
    });

    m_builder.getGsiBuilder().addPublicSymbols(std::move(publics));
}

} // namespace di::object_file
