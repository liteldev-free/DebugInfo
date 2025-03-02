#pragma once

#include "data_format/type/magic_entry.h"
#include "io/streamed_io.h"

namespace di::data_format {

class MagicBlob : public io::StreamedIO {
public:
    using for_each_callback_t = std::function<void(hash_t, MagicEntry const&)>;

    void read(const fs::path& path) override;

    void for_each(const for_each_callback_t& callback) const {
        for (const auto& [hash, entry] : m_entries) callback(hash, *entry);
    }

    constexpr size_t count() const { return m_entries.size(); }

    MagicEntry const* query(std::string_view symbol) const;

private:
    std::unordered_map<hash_t, std::unique_ptr<MagicEntry>> m_entries;

    // MagicBlob uses a custom algorithm to transform the stored seed. When
    // querying, you should use m_query_seed.
    uint64_t m_stored_seed{};
    uint64_t m_query_seed{};
};

} // namespace di::data_format
