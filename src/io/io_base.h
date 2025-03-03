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

class UnableToOpenException : public UnixException {
public:
    explicit UnableToOpenException(const fs::path& path) {
        add_context("path", path.string());
    }

    constexpr std::string category() const { return "exception.io.cantopen"; }
};

} // namespace di::io
