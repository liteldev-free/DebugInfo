#pragma once

#include <nlohmann/json_fwd.hpp>

namespace di {

struct MagicEntry {
#if __cpp_aggregate_paren_init < 201902L
    MagicEntry(std::bitset<64> flags, rva_t rva) : flags(flags), rva(rva) {}
#endif

    std::bitset<64> flags;
    // What is stored in the original format is not the RVA itself, but the
    // difference with the previous entry (in MagicBlob, RVA is sorted from
    // small to large)
    // But here, we still store the "real" RVA.
    rva_t rva;
    // Do not put the original hash in the entry yet.
    // hash_t hash;

    constexpr bool is_function() const { return flags[0]; }
    constexpr bool _unk2() const { return flags[1]; }
    constexpr bool is_verbose() const { return flags[2]; }
    constexpr bool _unk4() const { return flags[3]; }
};

void to_json(nlohmann::json& json, const MagicEntry& entry);

// TODO
// void from_json(const nlohmann::json& json, MagicEntry& entry);

} // namespace di
