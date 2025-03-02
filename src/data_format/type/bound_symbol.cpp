#include "data_format/type/bound_symbol.h"

#include <nlohmann/json.hpp>

namespace di {

void to_json(nlohmann::json& json, const BoundSymbol& symbol) {
    json["symbol"]      = symbol.m_symbol_name;
    json["rva"]         = symbol.m_rva;
    json["is_function"] = symbol.m_is_function;
}

void from_json(const nlohmann::json& json, BoundSymbol& symbol) {
    symbol.m_symbol_name = json["symbol"];
    symbol.m_rva         = json["rva"];
    symbol.m_is_function = json["is_function"];
}

} // namespace di