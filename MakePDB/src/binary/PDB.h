#pragma once

#include "binary/COFF.h"
#include "symbol_data.h"

#include <llvm/DebugInfo/PDB/Native/PDBFileBuilder.h>
#include <llvm/Support/Allocator.h>

namespace makepdb::binary {

class PDB {
public:
    explicit PDB(COFF&& coff, SymbolData&& symbol_data);

    void write(std::string_view path);

private:
    void build();

    inline void build_Info();
    inline void build_DBI();
    inline void build_TPI();
    inline void build_GSI();

    COFF       m_owning_coff;
    SymbolData m_owning_symbol_data;

    uint64_t m_image_base;

    BumpPtrAllocator    m_allocator;
    pdb::PDBFileBuilder m_builder;
};

} // namespace makepdb::binary
