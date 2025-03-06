#include "data_format/typed_symbol_list.h"

using namespace di::io;

namespace di::data_format {

void TypedSymbolList::read(const fs::path& path) {
    std::ifstream ifs(path);
    if (!ifs) throw UnableToOpenException(path);

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;

        auto sep_pos = line.find(", ");
        if (sep_pos == std::string::npos) {
            throw MissingDeclTypeException(path, line);
        }

        auto decl_type_s = line.substr(0, sep_pos);
        auto symbol      = line.substr(sep_pos + 2);

        m_data.emplace(symbol, DeclType(decl_type_s));
    }
}

void TypedSymbolList::write(const fs::path& path) const {
    std::ofstream ofs(path);
    for (const auto& [symbol, decl_type] : m_data) {
        ofs << decl_type.string() << ", " << symbol << "\n";
    }
}

void TypedSymbolList::record(const std::string& symbol, DeclType type) {
    m_data.emplace(symbol, type);
}

} // namespace di::data_format
