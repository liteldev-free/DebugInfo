#pragma once

#include "binary/COFF.h"
#include "symbol_data.h"

#include <llvm/DebugInfo/PDB/Native/PDBFileBuilder.h>
#include <llvm/Support/Allocator.h>

namespace makepdb::binary {

class PDB {
public:
    explicit PDB(COFF&& COFF, SymbolData&& SymbolData);

    void WriteTo(std::string_view Path);

private:
    void Build();

    inline void BuildInfo();
    inline void BuildDBI();
    inline void BuildTPI();
    inline void BuildGSI();

    COFF       OwningCOFF;
    SymbolData OwningSymbolData;

    uint64_t ImageBase;

    BumpPtrAllocator    Allocator;
    pdb::PDBFileBuilder Builder;
};

} // namespace makepdb::binary
