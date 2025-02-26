#include "data_format/typed_symbol_list.h"

namespace di::data_format {

TypedSymbolList::TypedSymbolList(const std::vector<std::string_view>& paths) {
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

            m_data.emplace(symbol, DeclType(declType_s));
        }

        std::println("Read {} symbols from dumped symlist.", m_data.size());
    }
}

} // namespace di::data_format
