#include "object_file/coff.h"
#include "error.h"

namespace di::object_file {

COFF::COFF(const fs::path& path) {
    using namespace object;

    auto file = check_llvm_result(ObjectFile::createObjectFile(path.string()));

    if (!isa<COFFObjectFile>(file.getBinary()))
        throw UnexceptObjectException(path, TypeOnly<COFFObjectFile>{});

    auto bin = file.takeBinary();

    m_owning_binary = object::OwningBinary(
        static_unique_ptr_cast<COFFObjectFile>(std::move(bin.first)),
        std::move(bin.second)
    );
}

codeview::PDB70DebugInfo COFF::get_debug_info() const {
    const codeview::DebugInfo* debug_info;
    StringRef                  pdb_file_name;

    if (get_owning_coff().getDebugPDBInfo(debug_info, pdb_file_name)
        || !debug_info) {
        throw MissingPDBInfoException();
    }


    if (auto signature = debug_info->Signature.CVSignature;
        signature != OMF::Signature::PDB70) {
        throw UnsupportPDBFormatException(signature);
    }

    return debug_info->PDB70;
}

size_t COFF::get_section_index(size_t offset) const {
    using namespace object;

    size_t current_index{};
    for (const SectionRef& sec_ref : get_owning_coff().sections()) {
        const coff_section* section = get_owning_coff().getCOFFSection(sec_ref);
        if (offset >= section->VirtualAddress
            && offset < section->VirtualAddress + section->VirtualSize) {
            return current_index;
        }
        current_index++;
    }
    throw SectionNotFoundException(offset);
}

object::coff_section* COFF::get_section_table() {
    return reinterpret_cast<object::coff_section*>(
        get_owning_coff().section_begin()->getRawDataRefImpl().p
    );
}

uint32_t COFF::get_number_of_sections() const {
    return get_owning_coff().getNumberOfSections();
}

object::COFFObjectFile const& COFF::get_owning_coff() const {
    return *m_owning_binary.getBinary();
}

} // namespace di::object_file
