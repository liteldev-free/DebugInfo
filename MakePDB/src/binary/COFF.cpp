#include "binary/COFF.h"

namespace makepdb::binary {

COFF::COFF(std::string_view Path) {
    using namespace object;

    auto ObjOrErr = ObjectFile::createObjectFile(Path);
    if (!ObjOrErr) {
        throw std::runtime_error("Failed to create object file.");
    }

    if (!isa<COFFObjectFile>(ObjOrErr->getBinary())) {
        throw std::runtime_error("Is not a valid PE file.");
    }

    auto Bin = ObjOrErr->takeBinary();

    OwningBinary = object::OwningBinary(
        static_unique_ptr_cast<COFFObjectFile>(std::move(Bin.first)),
        std::move(Bin.second)
    );
}

codeview::PDB70DebugInfo COFF::DebugInfo() const {
    const codeview::DebugInfo* DebugInfo;
    StringRef                  PDBFileName;

    if (OwningCOFF().getDebugPDBInfo(DebugInfo, PDBFileName) || !DebugInfo) {
        throw std::runtime_error("Failed to get pdb info from coff file.");
    }

    if (DebugInfo->Signature.CVSignature != OMF::Signature::PDB70) {
        throw std::runtime_error("Unsupported PDB format.");
    }

    return DebugInfo->PDB70;
}

size_t COFF::SectionIndex(uint64_t Offset) const {
    using namespace object;

    uint64_t CurrentIndex = 0;
    for (const SectionRef& Sec : OwningCOFF().sections()) {
        const coff_section* Section = OwningCOFF().getCOFFSection(Sec);
        if (Offset >= Section->VirtualAddress
            && Offset < Section->VirtualAddress + Section->VirtualSize) {
            return CurrentIndex;
        }
        CurrentIndex++;
    }
    throw std::runtime_error("Offset is not in any section.");
}

object::COFFObjectFile const& COFF::OwningCOFF() const {
    return *OwningBinary.getBinary();
}

} // namespace makepdb::binary
