#pragma once

namespace di::data_format {

class IOBase {
public:
    virtual ~IOBase() = default;

    virtual void read(const std::filesystem::path& path)        = 0;
    virtual void write(const std::filesystem::path& path) const = 0;
};

} // namespace di::data_format
