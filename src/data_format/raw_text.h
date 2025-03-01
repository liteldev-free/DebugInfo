#pragma once

#include "io/io_base.h"

namespace di::data_format {

class RawText : public io::IOBase {
public:
    void read(const fs::path& path) override;
    void write(const fs::path& path) const override;

    void record(std::string_view line);

private:
    std::string m_data;
};

} // namespace di::data_format
