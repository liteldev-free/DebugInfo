#include "io/streamed_io.h"

namespace di {

void StreamedIO::read(const fs::path& path) {
    m_file_stream.open(path, std::ios::binary);
    if (!m_file_stream) {
        throw std::runtime_error("Failed to open file!");
    }
}

void StreamedIO::write(const fs::path& path) const {
    std::fstream ofs(path, std::ios::binary);
    ofs << m_file_stream.rdbuf();
}

} // namespace di