#pragma once

#include "data_format/type/decl_type.h"

namespace di {

struct TypedSymbol {
#if __cpp_aggregate_paren_init < 201902L
    TypedSymbol(std::string_view name, DeclType type)
    : m_name(name),
      m_type(type) {}
#endif

    std::string m_name;
    DeclType    m_type;

    constexpr bool operator==(const TypedSymbol& other) const {
        return m_name == other.m_name;
    }
};

} // namespace di

namespace std {

template <>
struct hash<di::TypedSymbol> {
    constexpr size_t operator()(const di::TypedSymbol& symbol) const {
        return std::hash<std::string>{}(symbol.m_name);
    }
};

} // namespace std