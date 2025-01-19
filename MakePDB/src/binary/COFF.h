#pragma once

#include <llvm/Object/COFF.h>

namespace makepdb::binary {

class COFF {
public:
    explicit COFF(std::string_view Path);

    codeview::PDB70DebugInfo DebugInfo() const;
    size_t                   SectionIndex(uint64_t Offset) const;

    object::COFFObjectFile const& OwningCOFF() const;

private:
    object::OwningBinary<object::COFFObjectFile> OwningBinary;
};

} // namespace makepdb::binary
