#include "data_format/typed_symbol_list.h"

namespace di::data_format {

void TypedSymbolList::read(const std::filesystem::path& path) {
    std::ifstream ifs(path);
    if (!ifs) {
        throw std::runtime_error("Failed to open symlist file.");
    }

    std::string line;
    while (std::getline(ifs, line)) {
        if (line.empty()) continue;

        auto sep_pos = line.find(", ");
        if (sep_pos == std::string::npos) {
            throw std::runtime_error(
                "Symbol data is not included declType, please re-generate "
                "symlist file with -record-decl-name."
            );
        }

        auto decl_type_s = line.substr(0, sep_pos);
        auto symbol      = line.substr(sep_pos + 2);

        m_data.emplace(symbol, DeclType(decl_type_s));
    }
}

void TypedSymbolList::write(const std::filesystem::path& path) const {
    std::ofstream ofs(path);
    for (const auto& [symbol, decl_type] : m_data) {
        ofs << symbol << ", " << decl_type.string() << "\n";
    }
}

void TypedSymbolList::record(const std::string& symbol, DeclType type) {
    m_data.emplace(symbol, type);
}

} // namespace di::data_format
