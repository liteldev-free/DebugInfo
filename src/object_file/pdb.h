#pragma once

#include "data_format/bound_symbol_list.h"
#include "io/io_base.h"
#include "object_file/coff.h"

#include <llvm/DebugInfo/CodeView/CVRecord.h>
#include <llvm/DebugInfo/CodeView/MergingTypeTableBuilder.h>
#include <llvm/DebugInfo/CodeView/SymbolRecord.h>
#include <llvm/DebugInfo/PDB/Native/NativeSession.h>
#include <llvm/DebugInfo/PDB/Native/PDBFileBuilder.h>
#include <llvm/Support/Allocator.h>

namespace di::object_file {

class PDB : public io::IOBase {
public:
    using owning_coff_t        = std::unique_ptr<COFF>;
    using owning_symbol_data_t = std::unique_ptr<data_format::BoundSymbolList>;

    using for_each_symbol_callback_t =
        std::function<void(codeview::PublicSym32 const&)>;
    using for_each_type_callback_t =
        std::function<void(codeview::TypeIndex, codeview::CVType)>;

    enum IterableStream { TPI, IPI, Public };

    void read(const fs::path& path) override;
    void write(const fs::path& path) const override {
        const_cast<PDB*>(this)->_write(path);
    }

    // r
    pdb::NativeSession& get_native_session();
    pdb::PDBFile&       get_pdb_file();

    template <IterableStream Stream, typename CallbackT>
    void for_each(const CallbackT& callback) const {
        if constexpr (Stream == TPI) {
            return const_cast<PDB*>(this)->_for_each_ipi(
                *m_storaged_Tpi,
                callback
            );
        }
        if constexpr (Stream == IPI) {
            return const_cast<PDB*>(this)->_for_each_ipi(
                *m_storaged_Ipi,
                callback
            );
        }
        if constexpr (Stream == Public) {
            return const_cast<PDB*>(this)->_for_each_public(callback);
        }
    }

    // w
    void set_coff_object(owning_coff_t coff_object) {
        m_owning_coff = std::move(coff_object);
        m_image_base  = m_owning_coff->get_owning_coff().getImageBase();
    }

    void set_symbol_data(owning_symbol_data_t symbol_data) {
        m_owning_symbol_data = std::move(symbol_data);
    }

private:
    void build();

    // helpers
    void _write(const fs::path& path);

    void _for_each_public(const for_each_symbol_callback_t& callback);
    void _for_each_ipi(
        codeview::MergingTypeTableBuilder& type_table,
        const for_each_type_callback_t&    callback
    ) {
        type_table.ForEachRecord(callback);
    }

    owning_coff_t        m_owning_coff;
    owning_symbol_data_t m_owning_symbol_data;

    addr_t m_image_base;

    BumpPtrAllocator m_Alloc;

    // w
    std::unique_ptr<pdb::PDBFileBuilder> m_builder;

    // r
    std::unique_ptr<pdb::IPDBSession>                  m_session;
    std::unique_ptr<codeview::MergingTypeTableBuilder> m_storaged_Ipi;
    std::unique_ptr<codeview::MergingTypeTableBuilder> m_storaged_Tpi;
};

} // namespace di::object_file
