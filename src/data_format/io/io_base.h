#pragma once

namespace di {

class IOBase {
public:
    virtual ~IOBase() = default;

    virtual void read(const fs::path& path)        = 0;
    virtual void write(const fs::path& path) const = 0;
};

} // namespace di
