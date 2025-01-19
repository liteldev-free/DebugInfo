#pragma once

namespace makepdb {

struct DataEntity {
    std::string SymbolName;
    uint64_t    RVA;
    bool        IsFunction;

    bool operator==(const DataEntity& other) const {
        return SymbolName == other.SymbolName && RVA == other.RVA
            && IsFunction == other.IsFunction;
    }

    struct H {
        size_t operator()(const DataEntity E) const {
            size_t h1 = std::hash<std::string>{}(E.SymbolName);
            size_t h2 = std::hash<uint64_t>{}(E.RVA);
            size_t h3 = std::hash<bool>{}(E.IsFunction);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
};

class Data {
public:
    explicit Data(std::string_view Path);

    void forEach(const std::function<void(DataEntity)> Callback);

private:
    std::unordered_set<DataEntity, DataEntity::H> Entities;
};

} // namespace makepdb
