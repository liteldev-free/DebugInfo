#pragma once

#include "data_format/io_base.h"

namespace di::data_format {

class RawText : public IOBase {
public:
    void read(const std::filesystem::path& path) override;
    void write(const std::filesystem::path& path) const override;

    void record(std::string_view line);

private:
    std::string m_data;
};

} // namespace di::data_format
