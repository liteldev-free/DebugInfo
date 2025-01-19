#pragma once

#include "binary/COFF.h"
#include "data.h"

#include <llvm/DebugInfo/PDB/Native/PDBFileBuilder.h>
#include <llvm/Support/Allocator.h>

namespace makepdb::binary {

class PDB {
public:
    explicit PDB(COFF&& COFF, Data&& SymbolData);

    void WriteTo(std::string_view Path);

private:
    void Build();

    inline void BuildInfo();
    inline void BuildDBI();
    inline void BuildTPI();
    inline void BuildGSI();

    COFF OwningCOFF;
    Data OwningSymbolData;

    uint64_t ImageBase;

    BumpPtrAllocator    Allocator;
    pdb::PDBFileBuilder Builder;
};

} // namespace makepdb::binary
