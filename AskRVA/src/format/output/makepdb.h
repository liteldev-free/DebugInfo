#pragma once

#include "format/output/all.h"

namespace format::output {

class OutputMakePDBFile : public DefaultOutputFile {
public:
    void record(std::string_view symbol, uint64_t rva, bool is_function) override;
    void save(std::string_view path) const override;

private:
    std::unordered_set<std::tuple<std::string, uint64_t, bool>> m_records;
};

} // namespace format::output
