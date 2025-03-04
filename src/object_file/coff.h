#pragma once

#include <llvm/Object/COFF.h>

namespace di::object_file {

class COFF {
public:
    explicit COFF(const fs::path& path);

    codeview::PDB70DebugInfo get_debug_info() const;

    size_t                get_section_index(size_t offset) const;
    object::coff_section* get_section_table();
    uint32_t              get_number_of_sections() const;

    object::COFFObjectFile const& get_owning_coff() const;

private:
    object::OwningBinary<object::COFFObjectFile> m_owning_binary;
};

class UnexceptObjectException : public LLVMException {
public:
    template <typename T>
    explicit UnexceptObjectException(const fs::path& path, TypeOnly<T>)
    : LLVMException("Unexpected ObjectFile!") {
        add_context("path", path.string());
        add_context("excepted", typeid(T).name());
    }

    constexpr std::string category() const {
        return "exception.llvm.invalidobject";
    }
};

class MissingPDBInfoException : public LLVMException {
public:
    explicit MissingPDBInfoException()
    : LLVMException("No PDB Info found in COFF file.") {}

    constexpr std::string category() const {
        return "exception.llvm.missingpdbinfo";
    }
};

class UnsupportPDBFormatException : public LLVMException {
public:
    explicit UnsupportPDBFormatException(uint32_t signature)
    : LLVMException("Unsupported PDB file format.") {
        add_context_v_hex("signature", signature);
    }

    constexpr std::string category() const {
        return "exception.llvm.missingpdbinfo";
    }
};

class SectionNotFoundException : public LLVMException {
public:
    explicit SectionNotFoundException(size_t offset)
    : LLVMException("The offset is not within any section.") {
        add_context_v_hex("offset", offset);
    }

    constexpr std::string category() const {
        return "exception.llvm.sectionnotfound";
    }
};

} // namespace di::object_file
