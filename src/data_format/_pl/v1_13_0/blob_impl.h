#pragma once

#include "data_format/magic_blob.h"

namespace di::data_format::_pl::v1_13_0 {

class MagicBlobImpl : public MagicBlob {
public:
    using for_each_callback_t =
        std::function<void(hash_t, shared_entry_t const&)>;

    void read(const fs::path& path) override;

    shared_entry_t query(std::string_view symbol) const override;

    void for_each(const for_each_callback_t& callback) const override;

    size_t count() const override;
};

} // namespace di::data_format::_pl::v1_13_0
