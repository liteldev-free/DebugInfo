#include "symbol_data.h"

#include <nlohmann/json.hpp>

namespace makepdb {

SymbolData::SymbolData(std::string_view Path) {
    std::ifstream IFS(Path.data());
    if (!IFS) {
        throw std::runtime_error("Failed to open data path.");
    }

    auto Data = nlohmann::json::parse(IFS);
    if (Data["version"] != 1) {
        throw std::runtime_error("Unsupported data version.");
    }

    for (const auto& E : Data["data"]) {
        Entities.emplace(
            SymbolDataEntity{E["symbol"], E["rva"], E["is_function"]}
        );
    }
}

void SymbolData::forEach(const std::function<void(SymbolDataEntity)> Callback) {
    for (const auto& E : Entities) Callback(E);
}

} // namespace makepdb
