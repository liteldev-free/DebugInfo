#include "data_format/human_readable_symbol_list.h"

namespace di::data_format {

void HumanReadableSymbolList::record(std::string_view symbol, uint64_t rva) {
    m_symbol_rva_map.try_emplace(std::string(symbol), rva);
}

void HumanReadableSymbolList::write_to(std::string_view path) const {
    std::ofstream ofs(path.data());
    if (!ofs) {
        throw std::runtime_error("Failed to open save file.");
    }

    for (const auto& [symbol, rva] : m_symbol_rva_map) {
        ofs << std::format("[{:#x}] {}\n", rva, symbol);
    }
}

} // namespace di::data_format
