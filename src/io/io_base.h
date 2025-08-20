#pragma once

namespace di::io {

class IOBase {
public:
    virtual ~IOBase() = default;

    virtual void read(const fs::path& path)        = 0;
    virtual void write(const fs::path& path) const = 0;
};

class IOException : public RuntimeException<IOException> {
public:
    using RuntimeException::RuntimeException;

    constexpr std::string category() const { return "exception.io"; }
};

class OutOfBoundException : public IOException {
public:
    explicit OutOfBoundException(size_t op, size_t buffer_size)
    : IOException("An attempt was made to move the read pointer outside the "
                  "buffer bounds.") {
        add_context_v_hex("op", op);
        add_context_v_hex("buffer_size", buffer_size);
    }

    constexpr std::string category() const { return "exception.io.outofbound"; }
};

class CorruptedVarIntException : public IOException {
public:
    explicit CorruptedVarIntException(std::string_view msg, size_t position)
    : IOException("{}", msg) {
        add_context_v_hex("position", position);
    }

    constexpr std::string category() const {
        return "exception.io.corruptedvarint";
    }
};

class UnableToOpenException : public UnixException {
public:
    explicit UnableToOpenException(const fs::path& path) {
        add_context("path", path.string());
    }

    constexpr std::string category() const { return "exception.io.cantopen"; }
};

} // namespace di::io
