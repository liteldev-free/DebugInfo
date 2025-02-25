#include "format/output/makepdb.h"

#include <nlohmann/json.hpp>

namespace format::output {

void OutputMakePDBFile::record(std::string_view symbol, uint64_t rva, bool is_function) {
    m_records.emplace(std::string(symbol), rva, is_function);
}

void OutputMakePDBFile::save(std::string_view path) const {
    std::ofstream ofs(path.data());
    if (!ofs) {
        throw std::runtime_error("Failed to open save file.");
    }

    nlohmann::json data;

    // MakePDB - Format V1
    data["version"] = 1;

    for (const auto& [symbol, rva, is_fun] : m_records) {
        data["data"].emplace_back(nlohmann::json{
            {"symbol",      symbol},
            {"rva",         rva   },
            {"is_function", is_fun}
        });
    }

    ofs << data.dump(4);
}

} // namespace format::output
