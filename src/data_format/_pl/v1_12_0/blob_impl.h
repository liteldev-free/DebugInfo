#pragma once

#include "data_format/magic_blob.h"

namespace di::data_format::_pl::v1_12_0 {

class MagicBlobImpl : public MagicBlob {
public:
    using for_each_callback_t =
        std::function<void(hash_t, shared_entry_t const&)>;

    void read(const fs::path& path) override;

    shared_entry_t query(std::string_view symbol) const override;

    void for_each(const for_each_callback_t& callback) const override {
        for (const auto& [hash, entry] : m_entries) callback(hash, entry);
    }

    size_t count() const override { return m_entries.size(); }

private:
    std::unordered_map<hash_t, shared_entry_t> m_entries;

    // MagicBlob uses a custom algorithm to transform the stored seed. When
    // querying, you should use m_query_seed.
    uint64_t m_stored_seed{};
    uint64_t m_query_seed{};
};

} // namespace di::data_format::_pl::v1_12_0
