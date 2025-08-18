#include "io/streamed_io.h"

namespace di::io {

void StreamedIO::read(const fs::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw UnableToOpenException(path);
    }

    auto size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    m_buffer.resize(size);
    if (!file.read(m_buffer.data(), size)) {
        throw IOException(
            "Failed to read {} bytes from {}.",
            size,
            path.string()
        );
    }

    m_position = 0; // reset
}

void StreamedIO::write(const fs::path& path) const {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        throw UnableToOpenException(path);
    }

    if (!file.write(m_buffer.data(), m_buffer.size())) {
        throw IOException(
            "Failed to write {} bytes to {}.",
            m_buffer.size(),
            path.string()
        );
    }
}

} // namespace di::io