#pragma once

#include "io/io_base.h"

#include "data_format/type/decl_type.h"
#include "data_format/type/typed_symbol.h"

namespace di::data_format {

class TypedSymbolList : public io::IOBase {
public:
    using for_each_callback_t = std::function<void(TypedSymbol const&)>;

    // this method in this class supports multiple calls (reading multiple
    // files)
    void read(const fs::path& path) override;
    void write(const fs::path& path) const override;

    void record(const std::string& symbol, DeclType type);

    constexpr void for_each(const for_each_callback_t& callback) const {
        for (const auto& entity : m_data) callback(entity);
    }

private:
    std::unordered_set<TypedSymbol> m_data;
};

class MissingDeclTypeException
: public RuntimeException<MissingDeclTypeException> {
public:
    explicit MissingDeclTypeException(
        const fs::path&  list_file_path,
        std::string_view current_line
    )
    : RuntimeException(
          "The symbol data file does not contain the decl_type. Please "
          "regenerate it using the -record-decl-name option."
      ) {
        add_context("path", list_file_path.string());
        add_context("current_line", current_line);
    }

    constexpr std::string category() const {
        return "exception.dataformat.missingdecltype";
    }
};

} // namespace di::data_format
