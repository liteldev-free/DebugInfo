#include "data_format/bound_symbol_list.h"

#include <nlohmann/json.hpp>

namespace di::data_format {

constexpr int BOUND_SYMBOL_LIST_FORMAT_VERSION = 1;

BoundSymbolList::BoundSymbolList(std::string_view path) {
    std::ifstream ifs(path.data());
    if (!ifs) {
        throw std::runtime_error("Failed to open data path.");
    }

    auto data = nlohmann::json::parse(ifs);
    if (data["version"] != BOUND_SYMBOL_LIST_FORMAT_VERSION) {
        throw std::runtime_error("Unsupported data version.");
    }

    for (const auto& entity : data["data"]) {
        m_entities.emplace(
            BoundSymbol{entity["symbol"], entity["rva"], entity["is_function"]}
        );
    }
}

void BoundSymbolList::record(
    std::string_view symbol,
    uint64_t         rva,
    bool             is_function
) {
    m_entities.emplace(BoundSymbol{std::string(symbol), rva, is_function});
}

void BoundSymbolList::write_to(const std::string& path) const {
    std::ofstream ofs(path);
    if (!ofs) {
        throw std::runtime_error("Failed to open file!");
    }

    nlohmann::json data;

    data["version"] = BOUND_SYMBOL_LIST_FORMAT_VERSION;
    for (const auto& entity : m_entities) {
        data["data"].emplace_back(nlohmann::json{
            {"symbol",      entity.m_symbol_name},
            {"rva",         entity.m_rva        },
            {"is_function", entity.m_is_function}
        });
    }

    ofs << data.dump(4);
}

} // namespace di::data_format
