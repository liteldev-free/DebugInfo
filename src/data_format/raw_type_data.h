#pragma once

#include "data_format/io_base.h"

#include <llvm/DebugInfo/CodeView/MergingTypeTableBuilder.h>

namespace di::data_format {

class RawTypeData : public IOBase {
public:
    using for_each_callback_t =
        std::function<void(codeview::TypeIndex, codeview::CVType)>;

    enum TypedStream { TPI, IPI };

    RawTypeData() : m_storaged_IPI(m_allocator), m_storaged_TPI(m_allocator) {}

    void read(const fs::path& path) override;
    void write(const fs::path& path) const override {
        throw std::runtime_error("Unsupported operation.");
    }

    template <TypedStream Stream>
    void for_each(const for_each_callback_t& callback) /*const*/ {
        if constexpr (Stream == TPI) {
            return m_storaged_TPI.ForEachRecord(callback);
        }
        if constexpr (Stream == IPI) {
            return m_storaged_IPI.ForEachRecord(callback);
        }
    }

private:
    BumpPtrAllocator m_allocator;

    codeview::MergingTypeTableBuilder m_storaged_IPI;
    codeview::MergingTypeTableBuilder m_storaged_TPI;
};

} // namespace di::data_format
