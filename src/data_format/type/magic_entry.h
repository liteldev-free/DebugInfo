#pragma once

#include <nlohmann/json_fwd.hpp>

namespace di {

struct MagicEntry {
    hash_t hash; // key
    rva_t  rva;  // value

    MagicEntry(hash_t hash, rva_t rva) : hash(hash), rva(rva) {}

    virtual void to_json(nlohmann::json& json) const;
};

} // namespace di
