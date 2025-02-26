#pragma once

#include <llvm/DebugInfo/CodeView/MergingTypeTableBuilder.h>
#include <llvm/DebugInfo/CodeView/TypeStreamMerger.h>

namespace makepdb {

class RawTypeData {
public:
    using ForEachTpiCallback =
        std::function<void(codeview::TypeIndex, codeview::CVType)>;

    enum TypedStream { TPI, IPI };

    explicit RawTypeData(std::string_view path);

    template <TypedStream Stream>
    void for_each(const ForEachTpiCallback& callback) /*const*/ {
        if constexpr (Stream == TPI) {
            return m_storaged_TPI.ForEachRecord(callback);
        }
        if constexpr (Stream == IPI) {
            return m_storaged_IPI.ForEachRecord(callback);
        }
    }

private:
    BumpPtrAllocator m_allocator;

    codeview::MergingTypeTableBuilder m_storaged_TPI;
    codeview::MergingTypeTableBuilder m_storaged_IPI;
};

} // namespace makepdb
