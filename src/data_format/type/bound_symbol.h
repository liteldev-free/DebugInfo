#pragma once

#include <boost/functional/hash.hpp>

namespace di {

struct BoundSymbol {
    std::string m_symbol_name;
    uint64_t    m_rva;
    bool        m_is_function;

    bool operator==(const BoundSymbol& other) const {
        return m_symbol_name == other.m_symbol_name && m_rva == other.m_rva
            && m_is_function == other.m_is_function;
    }
};

} // namespace di

namespace std {

template <>
struct hash<di::BoundSymbol> {
    constexpr size_t operator()(const di::BoundSymbol& symbol) const {
        size_t seed = 0;
        boost::hash_combine(seed, symbol.m_symbol_name);
        boost::hash_combine(seed, symbol.m_rva);
        boost::hash_combine(seed, symbol.m_is_function);
        return seed;
    }
};

} // namespace std
