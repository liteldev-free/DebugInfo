#pragma once

#include "data_format/type/bound_symbol.h"

namespace di::data_format {

class BoundSymbolList {
public:
    using for_each_callback_t = std::function<void(BoundSymbol const&)>;

    explicit BoundSymbolList() = default;
    explicit BoundSymbolList(std::string_view path);

    void record(std::string_view symbol, uint64_t rva, bool is_function);
    void write_to(const std::string& path) const;

    constexpr void for_each(const for_each_callback_t& callback) const {
        for (const auto& entity : m_entities) callback(entity);
    }

private:
    std::unordered_set<BoundSymbol> m_entities;
};

} // namespace di::data_format
