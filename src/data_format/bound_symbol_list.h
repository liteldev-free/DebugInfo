#pragma once

#include "data_format/io_base.h"
#include "data_format/type/bound_symbol.h"

namespace di::data_format {

class BoundSymbolList : public IOBase {
public:
    using for_each_callback_t = std::function<void(BoundSymbol const&)>;

    void read(const fs::path& path) override;
    void write(const fs::path& path) const override;

    void record(std::string_view symbol, rva_t rva, bool is_function);

    constexpr void for_each(const for_each_callback_t& callback) const {
        for (const auto& entity : m_entities) callback(entity);
    }

private:
    std::unordered_set<BoundSymbol> m_entities;
};

} // namespace di::data_format
