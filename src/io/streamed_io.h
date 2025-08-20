#pragma once

#include "io/io_base.h"

namespace di::io {

class StreamedIO : public io::IOBase {
public:
    explicit StreamedIO(std::string data) : m_buffer(std::move(data)) {}
    explicit StreamedIO(const fs::path& path) { read(path); }

    StreamedIO() = default;

    void read(const fs::path& path) override;
    void write(const fs::path& path) const override;

    template <TriviallyCopyable T>
    constexpr T get() {
        if (m_position + sizeof(T) > m_buffer.size()) {
            throw OutOfBoundException(sizeof(T), m_buffer.size());
        }
        T value;

        // To avoid violating the strict aliasing rule, so we use memcpy.
        std::memcpy(&value, m_buffer.data() + m_position, sizeof(T));
        m_position += sizeof(T);

        if constexpr (std::endian::native != std::endian::little) {
            return std::byteswap(value);
        } else {
            return value;
        }
    }

    template <std::unsigned_integral T>
    constexpr T get_varint() {
        T res = 0;

        int       shift     = 0;
        const int max_shift = sizeof(T) * 8;

        while (true) {
            if (m_position >= m_buffer.size()) {
                throw CorruptedVarIntException(
                    "Incomplete VarInt at end of buffer.",
                    m_position
                );
            }
            if (shift >= max_shift) {
                throw CorruptedVarIntException(
                    "VarInt is too long for the requested type.",
                    m_position
                );
            }

            auto byte  = static_cast<std::byte>(m_buffer[m_position++]);
            res       |= static_cast<T>(byte & std::byte{0x7F}) << shift;

            if ((byte & std::byte{0x80}) == std::byte{0}) {
                break;
            }

            shift += 7;
        }
        return res;
    }

    template <TriviallyCopyable T>
    constexpr void put(T value) {
        if constexpr (std::endian::native != std::endian::little) {
            value = std::byteswap(value);
        }

        auto data = reinterpret_cast<const char*>(&value);
        if (m_position + sizeof(T) > m_buffer.size()) {
            m_buffer.append(data, sizeof(T));
        } else {
            std::memcpy(m_buffer.data() + m_position, data, sizeof(T));
        }

        m_position += sizeof(T);
    }

    template <std::unsigned_integral T>
    constexpr void put_varint(T value) {
        while (value >= 0x80) {
            std::byte to_write = static_cast<std::byte>((value & 0x7F) | 0x80);
            put(static_cast<char>(to_write));
            value >>= 7;
        }
        put(static_cast<char>(value));
    }

    constexpr void seek(size_t pos) {
        if (pos > m_buffer.size()) {
            throw OutOfBoundException(pos, m_buffer.size());
        }
        m_position = pos;
    }

    size_t size() const { return m_buffer.size(); }

    bool is_eof() const { return m_position >= m_buffer.size(); }

    constexpr size_t position() const { return m_position; }

    std::string& underlying() { return m_buffer; }

private:
    std::string m_buffer;
    size_t      m_position;
};

} // namespace di::io
