#pragma once

#include "data_format/magic_blob.h"

namespace di::data_format::_pl::v1_13_0 {

class MagicBlobImpl : public MagicBlob {
public:
    MagicBlobImpl() = default;

    void read(const fs::path& path) override;

    shared_entry_t query(std::string_view symbol) const override {
        return std::make_shared<MagicEntry>(0, 0);
    }

    void for_each(const for_each_callback_t& callback) const override {}

    size_t count() const override { return 0; }
};

} // namespace di::data_format::_pl::v1_13_0
