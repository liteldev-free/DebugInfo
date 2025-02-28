#pragma once

namespace di {

struct MagicEntry {
    std::bitset<64> flags;
    // What is stored in the original format is not the RVA itself, but the
    // difference with the previous entry (in MagicBlob, RVA is sorted from
    // small to large)
    // But here, we still store the "real" RVA.
    rva_t rva;
    // Do not put the original hash in the entry yet.
    // hash_t hash;

    constexpr bool is_function() { return flags[0]; }
    constexpr bool _unk2() { return flags[1]; }
    constexpr bool is_verbose() { return flags[2]; }
    constexpr bool _unk4() { return flags[3]; }
};

} // namespace di
