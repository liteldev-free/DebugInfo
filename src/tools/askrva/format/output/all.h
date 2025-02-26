#pragma once

namespace format {

enum class OutputFormat { Text, FakePDB, MakePDB };

namespace output {

class IOutputFile {
public:
    virtual ~IOutputFile() = default;

    virtual void record(std::string_view symbol, uint64_t rva, bool is_function) = 0;
    virtual void save(std::string_view path) const                               = 0;

    virtual void record_failure(std::string_view symbol)   = 0;
    virtual void save_failure(std::string_view path) const = 0;
};

class DefaultOutputFile : public IOutputFile {
public:
    void record_failure(std::string_view symbol) override;
    void save_failure(std::string_view path) const override;

protected:
    std::unordered_set<std::string> m_failed_symbols;
};

std::unique_ptr<IOutputFile> create(OutputFormat format);

} // namespace output

} // namespace format
