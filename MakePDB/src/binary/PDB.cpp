#include "binary/PDB.h"

#include "llvm/DebugInfo/MSF/MSFBuilder.h"
#include "llvm/DebugInfo/PDB/Native/DbiStreamBuilder.h"
#include "llvm/DebugInfo/PDB/Native/GSIStreamBuilder.h"
#include "llvm/DebugInfo/PDB/Native/InfoStreamBuilder.h"
#include "llvm/DebugInfo/PDB/Native/TpiStreamBuilder.h"
#include <llvm/DebugInfo/PDB/Native/RawConstants.h>

using namespace llvm::pdb;

namespace makepdb::binary {

PDB::PDB(COFF&& coff, SymbolData&& symbol_data)
: m_owning_coff(std::move(coff)),
  m_owning_symbol_data(std::move(symbol_data)),
  m_builder(m_allocator) {
    constexpr uint32_t block_size = 4096;
    if (m_builder.initialize(block_size)) {
        throw std::runtime_error("Failed to initialize pdb file builder.");
    }

    for (uint32_t I = 0; I < pdb::kSpecialStreamCount; ++I) {
        if (!m_builder.getMsfBuilder().addStream(0)) {
            throw std::runtime_error("Failed to add initial stream.");
        }
    }

    m_image_base = m_owning_coff.get_owning_coff().getImageBase();
}

void PDB::write(std::string_view path) {
    build();

    auto guid = m_builder.getInfoBuilder().getGuid();
    if (m_builder.commit(path, &guid)) {
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
    auto  pdb_info     = m_owning_coff.get_debug_info();
    auto& info_builder = m_builder.getInfoBuilder();

    info_builder.setVersion(PdbRaw_ImplVer::PdbImplVC70);
    info_builder.setAge(pdb_info.Age);
    info_builder.setGuid(*reinterpret_cast<codeview::GUID*>(pdb_info.Signature)
    );
    info_builder.addFeature(PdbRaw_FeatureSig::VC140);
}

void PDB::build_DBI() {
    auto  pdb_info = m_owning_coff.get_debug_info();
    auto& DBI      = m_builder.getDbiBuilder();

    DBI.setVersionHeader(PdbRaw_DbiVer::PdbDbiV70);
    DBI.setAge(pdb_info.Age);
    DBI.setMachineType(PDB_Machine::Amd64);
    DBI.setFlags(DbiFlags::FlagStrippedMask);
    DBI.setBuildNumber(14, 11); // LLVM is compatible with LINK 14.11

    // Add sections.
    auto section_table      = m_owning_coff.get_section_table();
    auto number_of_sections = m_owning_coff.get_number_of_sections();

    auto section_data_ref = ArrayRef<uint8_t>(
        (uint8_t*)section_table,
        m_owning_coff.get_number_of_sections() * sizeof(object::coff_section)
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
    m_builder.getTpiBuilder().setVersionHeader(PdbRaw_TpiVer::PdbTpiV80);
    m_builder.getIpiBuilder().setVersionHeader(PdbRaw_TpiVer::PdbTpiV80);
}

void PDB::build_GSI() {
    std::vector<BulkPublic> publics;
    m_owning_symbol_data.for_each([&publics,
                                   this](const SymbolDataEntity& entity) {
        BulkPublic symbol;

        auto section_index =
            m_owning_coff.get_section_index(entity.rva - m_image_base);
        auto section_or_err =
            m_owning_coff.get_owning_coff().getSection(section_index + 1);
        if (!section_or_err) {
            throw std::runtime_error("Invalid section.");
        }

        symbol.Name    = strdup(entity.symbol_name.c_str());
        symbol.NameLen = entity.symbol_name.size();
        symbol.Segment = section_index + 1;
        symbol.Offset =
            entity.rva - m_image_base - section_or_err.get()->VirtualAddress;
        if (entity.is_function)
            symbol.setFlags(codeview::PublicSymFlags::Function);

        publics.emplace_back(symbol);
    });

    m_builder.getGsiBuilder().addPublicSymbols(std::move(publics));
}

} // namespace makepdb::binary
