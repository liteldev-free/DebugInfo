#pragma once

#include "io/io_base.h"

namespace di {

class StreamedIO : public IOBase {
public:
    void read(const fs::path& path) override;
    void write(const fs::path& path) const override;

    constexpr auto next() { return m_file_stream.peek(); }

    // Avoid confusion with the function that opens a file, so use eat
    // instead of read. Maybe come up with another name for write as well.
    //
    // TODO: write<T>() method.

    template <typename T>
    constexpr T eat() {
        T value;
        m_file_stream.read((char*)&value, sizeof(T));
        return value;
    }

    template <std::unsigned_integral T>
    constexpr T eat_varint() {
        T   res   = 0;
        int shift = 0;
        while (true) {
            auto byte  = m_file_stream.get();
            res       |= static_cast<T>(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) break;
            shift += 7;
        }
        return res;
    }

private:
    std::ifstream m_file_stream;
};

} // namespace di
