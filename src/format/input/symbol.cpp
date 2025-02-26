#include "format/input/symbol.h"

namespace format::input {

SymbolListFile SymbolListFile::load(std::string_view path) {
    return load(std::vector<std::string>{path.data()});
}

SymbolListFile SymbolListFile::load(const std::vector<std::string>& paths) {
    SymbolListFile result;

    for (const auto& path : paths) {
        std::ifstream ifs(path.data());
        if (!ifs) {
            throw std::runtime_error("Failed to open symlist file.");
        }

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;

            auto separator_pos = line.find(", ");
            if (separator_pos == std::string::npos) {
                throw std::runtime_error(
                    "Symbol data is not included declType, please re-generate "
                    "symlist file with -record-decl-name."
                );
            }

            auto declType_s = line.substr(0, separator_pos);
            auto symbol     = line.substr(separator_pos + 2);

            result.m_data.emplace(symbol, DeclType(declType_s));
        }

        std::println(
            "Read {} symbols from dumped symlist.",
            result.m_data.size()
        );
    }

    return result;
}

void SymbolListFile::for_each(const std::function<void(Symbol)>& callback) {
    for (const auto& entity : m_data) callback(entity);
}

} // namespace format::input
