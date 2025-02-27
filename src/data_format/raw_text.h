#pragma once

namespace di::data_format {

class RawText {
public:
    void record(std::string_view line);
    void write_to(std::string_view path) const;

private:
    std::string m_data;
};

} // namespace di::data_format
