#include "blob_impl.h"
#include "entry_impl.h"

#define XXH_INLINE_ALL
#include "xxhash.h"

namespace {

constexpr uint64_t hash_qseed(uint64_t stored_seed) {
    constexpr auto v1 = 0x7ED55D16u;
    constexpr auto v2 = 0xC761C23Cu;
    constexpr auto v3 = 0x165667B1u;
    constexpr auto v4 = 0x160733E3u;
    constexpr auto v5 = 0x028FB93Bu;
    constexpr auto v6 = 0xB55A4F09uLL;

    constexpr auto al = [](uint32_t a) {
        auto c1 = (0x1000 + 1) * a + v1;
        auto d1 = (0x20 + 1) * (c1 ^ (c1 >> 19) ^ v2);
        auto e1 = ((d1 + v3) << 9) ^ (d1 - v4);
        return e1 + 8 * e1 - v5;
    };

    auto a1 = al(stored_seed >> 32);
    auto a2 = al(stored_seed);

    auto b1 = v6 | 0xFFFFFFFF00000000uLL;

    auto c1 = ((a1 & 0xFFFF0000) ^ ((a1 ^ b1) << 16)) << 16;
    auto c2 = a2 ^ ((a2 ^ v6 << 16) >> 16);

    return c1 | c2;
}

} // namespace

namespace di::data_format::_pl::v1_12_0 {

void MagicBlobImpl::read(const fs::path& path) {
    StreamedIO::read(path);

    m_stored_seed = eat<uint64_t>();
    m_query_seed  = hash_qseed(m_stored_seed);

    rva_t n_rva{};

    while (next() != EOF) {
        auto flags = eat_varint<uint64_t>();
        auto rva   = eat_varint<rva_t>();
        auto hash  = eat<hash_t>();

        // What is stored in the original format is not the RVA itself, but the
        // difference with the previous entry (in MagicBlob, RVA is sorted from
        // small to large)
        // But here, we still store the "real" RVA.
        n_rva += rva;
        rva    = n_rva;

        m_entries.emplace(
            hash,
            std::make_shared<MagicEntryImpl>(hash, rva, flags)
        );
    }
}

MagicBlob::shared_entry_t MagicBlobImpl::query(std::string_view symbol) const {
    auto query_hash = XXH64(symbol.data(), symbol.size(), m_query_seed);
    if (m_entries.contains(query_hash)) {
        return m_entries.at(query_hash);
    }
    return nullptr;
}

} // namespace di::data_format::_pl::v1_12_0
