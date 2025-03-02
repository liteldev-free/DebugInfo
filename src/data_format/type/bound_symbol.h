#pragma once

#include <nlohmann/json_fwd.hpp>

namespace di {

struct BoundSymbol {
    std::string m_symbol_name;
    rva_t       m_rva;
    bool        m_is_function;

    bool operator==(const BoundSymbol& other) const {
        return m_symbol_name == other.m_symbol_name && m_rva == other.m_rva
            && m_is_function == other.m_is_function;
    }
};

void to_json(nlohmann::json& json, const BoundSymbol& symbol);
void from_json(const nlohmann::json& json, BoundSymbol& symbol);

} // namespace di

namespace std {

template <>
struct hash<di::BoundSymbol> {
    constexpr size_t operator()(const di::BoundSymbol& symbol) const {
        size_t seed = 0;
        hash_combine(seed, symbol.m_symbol_name);
        hash_combine(seed, symbol.m_rva);
        hash_combine(seed, symbol.m_is_function);
        return seed;
    }
};

} // namespace std
