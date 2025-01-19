#include "format/output/fakepdb.h"

#include <nlohmann/json.hpp>

namespace format::output {

void OutputFakePDBFile::save(std::string_view path) const {
    std::ofstream ofs(path.data());
    if (!ofs) {
        throw std::runtime_error("Failed to open save file.");
    }

    nlohmann::json data;
    for (const auto& [symbol, rva] : m_symbol_rva_map) {
        data[symbol] = std::format("{:#x}", rva);
    }

    ofs << data.dump(4);
}

} // namespace format::output
