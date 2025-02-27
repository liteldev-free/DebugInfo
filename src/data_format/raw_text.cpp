#include "data_format/raw_text.h"

namespace di::data_format {

void RawText::record(std::string_view content) {
    m_data += content;
    m_data += '\n';
}

void RawText::write_to(std::string_view path) const {
    std::ofstream ofs(path.data());
    if (!ofs) {
        throw std::runtime_error("Failed to open save file.");
    }

    ofs << m_data;
}

} // namespace di::data_format
