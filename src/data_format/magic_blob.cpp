#include "data_format/magic_blob.h"

#include "data_format/_pl/v1_12_0/blob_impl.h"
#include "data_format/_pl/v1_13_0/blob_impl.h"

namespace di::data_format {

MagicBlob::blob_t MagicBlob::create(FormatVersion version) {
    switch (version) {
    case V1_12_0:
        return std::make_unique<_pl::v1_12_0::MagicBlobImpl>();
    case V1_13_0:
        return std::make_unique<_pl::v1_13_0::MagicBlobImpl>();
    }
    return nullptr;
}

} // namespace di::data_format