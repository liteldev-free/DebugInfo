#pragma once

#include "data_format/io/streamed_io.h"
#include "data_format/type/magic_entry.h"

namespace di::data_format {

class MagicBlob : public StreamedIO {
public:
    void read(const fs::path& path) override;

    constexpr size_t count() const { return m_entities.size(); }

    MagicEntry const* query(std::string_view symbol) const;

private:
    std::unordered_map<hash_t, std::unique_ptr<MagicEntry>> m_entities;

    // MagicBlob uses a custom algorithm to transform the stored seed. When
    // querying, you should use m_query_seed.
    uint64_t m_stored_seed{};
    uint64_t m_query_seed{};
};

} // namespace di::data_format
