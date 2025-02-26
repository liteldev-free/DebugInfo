#include "format/output/text.h"

namespace format::output {

void OutputTextFile::record(
    std::string_view symbol,
    uint64_t         rva,
    bool             is_function
) {
    m_symbol_rva_map.try_emplace(std::string(symbol), rva);
}

void OutputTextFile::save(std::string_view path) const {
    std::ofstream ofs(path.data());
    if (!ofs) {
        throw std::runtime_error("Failed to open save file.");
    }

    for (const auto& [symbol, rva] : m_symbol_rva_map) {
        ofs << std::format("[{:#x}] {}\n", rva, symbol);
    }
}

} // namespace format::output
