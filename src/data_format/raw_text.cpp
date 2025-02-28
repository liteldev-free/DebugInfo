#include "data_format/raw_text.h"

namespace di::data_format {

void RawText::read(const fs::path& path) {
    std::ifstream ifs(path);

    auto size = std::filesystem::file_size(path);
    m_data.reserve(size);
    m_data.assign(std::istreambuf_iterator<char>(ifs), {});
}

void RawText::write(const fs::path& path) const {
    std::ofstream ofs(path);
    if (!ofs) {
        throw std::runtime_error("Failed to open save file.");
    }

    ofs << m_data;
}

void RawText::record(std::string_view content) {
    m_data += content;
    m_data += '\n';
}

} // namespace di::data_format
