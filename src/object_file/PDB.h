#pragma once

#include "object_file/COFF.h"

#include "data_format/bound_symbol_list.h"
#include "data_format/raw_type_data.h"

#include <llvm/DebugInfo/PDB/Native/PDBFileBuilder.h>
#include <llvm/Support/Allocator.h>

namespace di::object_file {

class PDB {
public:
    using owning_coff_t        = std::unique_ptr<COFF>;
    using owning_symbol_data_t = std::unique_ptr<data_format::BoundSymbolList>;
    using owning_type_data_t   = std::unique_ptr<data_format::RawTypeData>;

    explicit PDB();

    void set_coff_object(owning_coff_t coff_object) {
        m_owning_coff = std::move(coff_object);
        m_image_base  = m_owning_coff->get_owning_coff().getImageBase();
    }

    void set_symbol_data(owning_symbol_data_t symbol_data) {
        m_owning_symbol_data = std::move(symbol_data);
    }
    void set_raw_type_data(owning_type_data_t raw_type_data) {
        m_owning_raw_type_data = std::move(raw_type_data);
    }

    void write(std::string_view path);

private:
    void build();

    inline void build_Info();
    inline void build_DBI();
    inline void build_TPI();
    inline void build_GSI();

    owning_coff_t        m_owning_coff;
    owning_symbol_data_t m_owning_symbol_data;
    owning_type_data_t   m_owning_raw_type_data;

    uint64_t m_image_base;

    BumpPtrAllocator    m_allocator;
    pdb::PDBFileBuilder m_builder;
};

} // namespace di::object_file
