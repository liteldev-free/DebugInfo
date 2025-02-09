#pragma once

namespace makepdb {

struct SymbolDataEntity {
    std::string SymbolName;
    uint64_t    RVA;
    bool        IsFunction;

    bool operator==(const SymbolDataEntity& other) const {
        return SymbolName == other.SymbolName && RVA == other.RVA
            && IsFunction == other.IsFunction;
    }

    struct H {
        size_t operator()(const SymbolDataEntity E) const {
            size_t h1 = std::hash<std::string>{}(E.SymbolName);
            size_t h2 = std::hash<uint64_t>{}(E.RVA);
            size_t h3 = std::hash<bool>{}(E.IsFunction);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
};

class SymbolData {
public:
    explicit SymbolData(std::string_view Path);

    void forEach(const std::function<void(SymbolDataEntity)> Callback);

private:
    std::unordered_set<SymbolDataEntity, SymbolDataEntity::H> Entities;
};

} // namespace makepdb
