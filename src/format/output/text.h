#pragma once

#include "format/output/all.h"

namespace format::output {

class OutputTextFile : public DefaultOutputFile {
public:
    void
    record(std::string_view symbol, uint64_t rva, bool is_function) override;
    void save(std::string_view path) const override;

protected:
    std::unordered_map<std::string, uint64_t> m_symbol_rva_map;
};

} // namespace format::output
