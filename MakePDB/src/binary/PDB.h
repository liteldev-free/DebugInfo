#pragma once

#include "binary/COFF.h"

#include "raw_type_data.h"
#include "symbol_data.h"

#include <llvm/DebugInfo/PDB/Native/PDBFileBuilder.h>
#include <llvm/Support/Allocator.h>

namespace makepdb::binary {

class PDB {
public:
    using OwningCOFF        = std::unique_ptr<COFF>;
    using OwningSymbolData  = std::unique_ptr<SymbolData>;
    using OwningRawTypeData = std::unique_ptr<RawTypeData>;

    explicit PDB();

    void set_coff_object(OwningCOFF coff_object) {
        m_owning_coff = std::move(coff_object);
        m_image_base  = m_owning_coff->get_owning_coff().getImageBase();
    }

    void set_symbol_data(OwningSymbolData symbol_data) {
        m_owning_symbol_data = std::move(symbol_data);
    }
    void set_raw_type_data(OwningRawTypeData raw_type_data) {
        m_owning_raw_type_data = std::move(raw_type_data);
    }

    void write(std::string_view path);

private:
    void build();

    inline void build_Info();
    inline void build_DBI();
    inline void build_TPI();
    inline void build_GSI();

    OwningCOFF        m_owning_coff;
    OwningSymbolData  m_owning_symbol_data;
    OwningRawTypeData m_owning_raw_type_data;

    uint64_t m_image_base;

    BumpPtrAllocator    m_allocator;
    pdb::PDBFileBuilder m_builder;
};

} // namespace makepdb::binary
