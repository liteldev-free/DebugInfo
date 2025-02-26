#pragma once

namespace di::data_format {

class HumanReadableSymbolList {
public:
    void record(std::string_view symbol, uint64_t rva);
    void write_to(std::string_view path) const;

protected:
    std::unordered_map<std::string, uint64_t> m_symbol_rva_map;
};

} // namespace di::data_format
