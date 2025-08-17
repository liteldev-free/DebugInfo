#include "entry_impl.h"

#include <nlohmann/json.hpp>

namespace di::data_format::_pl::v1_12_0 {

void MagicEntryImpl::to_json(nlohmann::json& json) const {
    MagicEntry::to_json(json);

    json["is_function"] = is_function();
    json["_unk2"]       = _unk2();
    json["is_verbose"]  = is_verbose();
    json["_unk4"]       = _unk4();
}

} // namespace di::data_format::_pl::v1_12_0