#include "data_format/bound_symbol_list.h"

#include <nlohmann/json.hpp>

namespace di::data_format {

void BoundSymbolList::read(const fs::path& path) {
    std::ifstream ifs(path);
    if (!ifs) {
        throw std::runtime_error("Failed to open data path.");
    }

    auto data = nlohmann::json::parse(ifs);

    m_entities.clear();
    for (const auto& entity : data["data"]) {
        m_entities.emplace(entity);
    }
}

void BoundSymbolList::write(const fs::path& path) const {
    std::ofstream ofs(path);
    if (!ofs) {
        throw std::runtime_error("Failed to open file!");
    }

    nlohmann::json data;
    for (const auto& entity : m_entities) {
        data.emplace_back(entity);
    }

    ofs << data.dump(4);
}

void BoundSymbolList::record(
    const std::string& symbol,
    rva_t              rva,
    bool               is_function
) {
    m_entities.emplace(BoundSymbol{symbol, rva, is_function});
}

} // namespace di::data_format
