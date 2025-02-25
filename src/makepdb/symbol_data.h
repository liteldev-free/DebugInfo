#pragma once

namespace makepdb {

struct SymbolDataEntity {
    std::string symbol_name;
    uint64_t    rva;
    bool        is_function;

    bool operator==(const SymbolDataEntity& other) const {
        return symbol_name == other.symbol_name && rva == other.rva
            && is_function == other.is_function;
    }

    struct Hash {
        size_t operator()(const SymbolDataEntity entity) const {
            size_t h1 = std::hash<std::string>{}(entity.symbol_name);
            size_t h2 = std::hash<uint64_t>{}(entity.rva);
            size_t h3 = std::hash<bool>{}(entity.is_function);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
};

class SymbolData {
public:
    explicit SymbolData(std::string_view path);

    void for_each(const std::function<void(SymbolDataEntity)> callback) const;

private:
    std::unordered_set<SymbolDataEntity, SymbolDataEntity::Hash> m_entities;
};

} // namespace makepdb
