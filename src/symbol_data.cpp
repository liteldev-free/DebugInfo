#include "symbol_data.h"

#include <nlohmann/json.hpp>

namespace makepdb {

SymbolData::SymbolData(std::string_view path) {
    std::ifstream ifs(path.data());
    if (!ifs) {
        throw std::runtime_error("Failed to open data path.");
    }

    auto data = nlohmann::json::parse(ifs);
    if (data["version"] != 1) {
        throw std::runtime_error("Unsupported data version.");
    }

    for (const auto& entity : data["data"]) {
        m_entities.emplace(SymbolDataEntity{
            entity["symbol"],
            entity["rva"],
            entity["is_function"]
        });
    }
}

void SymbolData::for_each(const std::function<void(SymbolDataEntity)> callback
) const {
    for (const auto& entity : m_entities) callback(entity);
}

} // namespace makepdb
