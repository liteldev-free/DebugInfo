#include "data_format/type/magic_entry.h"

#include <nlohmann/json.hpp>

namespace di {

void to_json(nlohmann::json& json, const MagicEntry& entry) {
    json["rva"]         = entry.rva;
    json["is_function"] = entry.is_function();
    json["_unk2"]       = entry._unk2();
    json["is_verbose"]  = entry.is_verbose();
    json["_unk4"]       = entry._unk4();
}

} // namespace di