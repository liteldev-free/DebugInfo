#include "data_format/type/magic_entry.h"

#include <nlohmann/json.hpp>

namespace di {

void MagicEntry::to_json(nlohmann::json& json) const {
    json["hash"] = hash;
    json["rva"]  = rva;
}

} // namespace di