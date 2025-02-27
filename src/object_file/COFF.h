#pragma once

#include <llvm/Object/COFF.h>

namespace di::object_file {

class COFF {
public:
    explicit COFF(std::string_view path);

    codeview::PDB70DebugInfo get_debug_info() const;

    size_t                get_section_index(uint64_t offset) const;
    object::coff_section* get_section_table();
    uint32_t              get_number_of_sections() const;

    object::COFFObjectFile const& get_owning_coff() const;

private:
    object::OwningBinary<object::COFFObjectFile> m_owning_binary;
};

} // namespace di::object_file
