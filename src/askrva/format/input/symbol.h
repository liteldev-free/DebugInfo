#pragma once

#include "format/input/decl_type.h"

namespace format::input {

class SymbolListFile {
public:
    [[nodiscard]] static SymbolListFile load(std::string_view path);
    [[nodiscard]] static SymbolListFile load(const std::vector<std::string>& path);

    void for_each(const std::function<void(Symbol)>& callback);

private:
    SymbolListFile() = default;

    std::unordered_set<Symbol> m_data;
};

} // namespace format::input
