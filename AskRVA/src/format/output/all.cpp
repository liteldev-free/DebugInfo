#include "format/output/all.h"

#include "format/output/fakepdb.h"
#include "format/output/makepdb.h"
#include "format/output/text.h"

namespace format::output {

void DefaultOutputFile::record_failure(std::string_view symbol) { m_failed_symbols.emplace(symbol); }

void DefaultOutputFile::save_failure(std::string_view path) const {
    std::ofstream ofs(path.data());
    if (!ofs) {
        throw std::runtime_error("Failed to open save file.");
    }

    for (const auto& symbol : m_failed_symbols) {
        ofs << symbol << "\n";
    }
}

std::unique_ptr<IOutputFile> create(OutputFormat format) {
    switch (format) {
    case OutputFormat::Text:
        return std::make_unique<OutputTextFile>();
    case OutputFormat::FakePDB:
        return std::make_unique<OutputFakePDBFile>();
    case OutputFormat::MakePDB:
        return std::make_unique<OutputMakePDBFile>();
    default:
        throw std::invalid_argument("Invalid output file format.");
    }
}

} // namespace format::output