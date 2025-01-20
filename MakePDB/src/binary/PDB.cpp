#include "binary/PDB.h"

#include "llvm/DebugInfo/MSF/MSFBuilder.h"
#include "llvm/DebugInfo/PDB/Native/DbiStreamBuilder.h"
#include "llvm/DebugInfo/PDB/Native/GSIStreamBuilder.h"
#include "llvm/DebugInfo/PDB/Native/InfoStreamBuilder.h"
#include "llvm/DebugInfo/PDB/Native/TpiStreamBuilder.h"
#include <llvm/DebugInfo/PDB/Native/RawConstants.h>

using namespace llvm::pdb;

namespace makepdb::binary {

PDB::PDB(COFF&& COFF, Data&& SymbolData)
: OwningCOFF(std::move(COFF)),
  OwningSymbolData(std::move(SymbolData)),
  Builder(Allocator) {
    constexpr uint32_t BlockSize = 4096;
    if (Builder.initialize(BlockSize)) {
        throw std::runtime_error("Failed to initialize pdb file builder.");
    }

    for (uint32_t I = 0; I < pdb::kSpecialStreamCount; ++I) {
        if (!Builder.getMsfBuilder().addStream(0)) {
            throw std::runtime_error("Failed to add initial stream.");
        }
    }

    ImageBase = OwningCOFF.OwningCOFF().getImageBase();
}

void PDB::WriteTo(std::string_view Path) {
    Build();

    auto Guid = Builder.getInfoBuilder().getGuid();
    if (Builder.commit(Path, &Guid)) {
        throw std::runtime_error("Failed to create pdb!");
    }
}

void PDB::Build() {
    BuildInfo();
    BuildDBI();
    BuildTPI();
    BuildGSI();
}

void PDB::BuildInfo() {
    auto  PDBInfo     = OwningCOFF.DebugInfo();
    auto& InfoBuilder = Builder.getInfoBuilder();

    InfoBuilder.setVersion(PdbRaw_ImplVer::PdbImplVC70);
    InfoBuilder.setAge(PDBInfo.Age);
    InfoBuilder.setGuid(*reinterpret_cast<codeview::GUID*>(PDBInfo.Signature));
    InfoBuilder.addFeature(PdbRaw_FeatureSig::VC140);
}

void PDB::BuildDBI() {
    auto  PDBInfo    = OwningCOFF.DebugInfo();
    auto& DbiBuilder = Builder.getDbiBuilder();

    DbiBuilder.setVersionHeader(PdbRaw_DbiVer::PdbDbiV70);
    DbiBuilder.setAge(PDBInfo.Age);
    DbiBuilder.setMachineType(PDB_Machine::Amd64);
    DbiBuilder.setFlags(DbiFlags::FlagStrippedMask);
    DbiBuilder.setBuildNumber(14, 11); // LLVM is compatible with LINK 14.11

    // Add sections.
    auto SectionTable     = OwningCOFF.SectionTable();
    auto NumberOfSections = OwningCOFF.NumberOfSections();

    auto SectionDataRef = ArrayRef<uint8_t>(
        (uint8_t*)SectionTable,
        OwningCOFF.NumberOfSections() * sizeof(object::coff_section)
    );

    auto SectionTableRef = ArrayRef<object::coff_section>(
        (const object::coff_section*)SectionDataRef.data(),
        NumberOfSections
    );

    DbiBuilder.createSectionMap(SectionTableRef);

    // Add COFF section header stream.
    if (DbiBuilder.addDbgStream(DbgHeaderType::SectionHdr, SectionDataRef)) {
        throw std::runtime_error("Failed to add dbg stream.");
    }
}

void PDB::BuildTPI() {
    Builder.getTpiBuilder().setVersionHeader(PdbRaw_TpiVer::PdbTpiV80);
    Builder.getIpiBuilder().setVersionHeader(PdbRaw_TpiVer::PdbTpiV80);
}

void PDB::BuildGSI() {
    std::vector<BulkPublic> PublicsIn;
    OwningSymbolData.forEach([&PublicsIn, this](const DataEntity& E) {
        BulkPublic Symbol;

        auto SectionIndex = OwningCOFF.SectionIndex(E.RVA - ImageBase);
        auto SectionOrErr =
            OwningCOFF.OwningCOFF().getSection(SectionIndex + 1);
        if (!SectionOrErr) {
            throw std::runtime_error("Invalid section.");
        }

        Symbol.Name    = strdup(E.SymbolName.c_str());
        Symbol.NameLen = E.SymbolName.size();
        Symbol.Segment = SectionIndex + 1;
        Symbol.Offset  = E.RVA - ImageBase - SectionOrErr.get()->VirtualAddress;
        if (E.IsFunction) Symbol.setFlags(codeview::PublicSymFlags::Function);

        PublicsIn.emplace_back(Symbol);
    });

    Builder.getGsiBuilder().addPublicSymbols(std::move(PublicsIn));
}

} // namespace makepdb::binary
