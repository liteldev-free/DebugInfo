#pragma once

#include "data_format/type/magic_entry.h"

#include <nlohmann/json_fwd.hpp>

namespace di::data_format::_pl::v1_12_0 {

struct MagicEntryImpl : public MagicEntry {
    using flags_t = std::bitset<64>;

    flags_t flags;

    MagicEntryImpl(hash_t hash, rva_t rva, flags_t flags)
    : MagicEntry(hash, rva),
      flags(flags) {}

    void to_json(nlohmann::json& json) const override;

    constexpr bool is_function() const { return flags[0]; }
    constexpr bool _unk2() const { return flags[1]; }
    constexpr bool is_verbose() const { return flags[2]; }
    constexpr bool _unk4() const { return flags[3]; }
};

// TODO
// void from_json(const nlohmann::json& json, MagicEntry& entry);

} // namespace di::data_format::_pl::v1_12_0
