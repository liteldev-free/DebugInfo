#pragma once

#include "data_format/type/magic_entry.h"
#include "io/streamed_io.h"

namespace di::data_format {

class MagicBlob : public io::StreamedIO {
public:
    enum FormatVersion {
        V1_12_0,
        V1_13_0,
    };

    using blob_t  = std::unique_ptr<MagicBlob>;
    using entry_t = std::unique_ptr<MagicEntry>;

    using shared_blob_t  = std::shared_ptr<const MagicBlob>;
    using shared_entry_t = std::shared_ptr<const MagicEntry>;

    using for_each_callback_t =
        std::function<void(hash_t, shared_entry_t const&)>;

    virtual void read(const fs::path& path) = 0;

    virtual shared_entry_t query(std::string_view symbol) const        = 0;
    virtual void   for_each(const for_each_callback_t& callback) const = 0;
    virtual size_t count() const                                       = 0;

    static blob_t create(FormatVersion version);

protected:
    MagicBlob() = default;
};

} // namespace di::data_format