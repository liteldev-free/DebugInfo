#pragma once

#include "data_format/type/typed_symbol.h"

namespace di::data_format {

class TypedSymbolList {
public:
    using for_each_callback_t = std::function<void(TypedSymbol const&)>;

    explicit TypedSymbolList(const std::string& path)
    : TypedSymbolList(std::vector<std::string>{path}) {};
    explicit TypedSymbolList(const std::vector<std::string>& paths);

    constexpr void for_each(const for_each_callback_t& callback) const {
        for (const auto& entity : m_data) callback(entity);
    }

private:
    std::unordered_set<TypedSymbol> m_data;
};

} // namespace di::data_format
