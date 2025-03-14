#include "magic_blob.h"

#define XXH_INLINE_ALL
#include "xxhash.h"

namespace {

constexpr uint64_t 立即抽卡(uint64_t 几连抽) {
    constexpr auto 神里绫华 = 0x7ED55D16u;
    constexpr auto 雷电将军 = 0xC761C23Cu;
    constexpr auto 八重神子 = 0x165667B1u;
    constexpr auto 荒泷一斗 = 0x160733E3u;
    constexpr auto 克洛琳德 = 0x028FB93Bu;
    constexpr auto 艾尔海森 = 0xB55A4F09uLL;

    constexpr auto 静水流涌之辉 = [](uint32_t 优菈) {
        auto 可莉 = (0x1000 + 1) * 优菈 + 神里绫华;
        auto 托马 = (0x20 + 1) * (可莉 ^ (可莉 >> 19) ^ 雷电将军);
        auto 宵宫 = ((托马 + 八重神子) << 9) ^ (托马 - 荒泷一斗);
        return 宵宫 + 8 * 宵宫 - 克洛琳德;
    };

    auto 甘雨   = 静水流涌之辉(几连抽 >> 32);
    auto 纳西妲 = 静水流涌之辉(几连抽);

    auto 心海 = 艾尔海森 | 0xFFFFFFFF00000000uLL;

    auto 原神 = ((甘雨 & 0xFFFF0000) ^ ((甘雨 ^ 心海) << 16)) << 16;
    auto 原魔 = 纳西妲 ^ ((纳西妲 ^ 艾尔海森 << 16) >> 16);

    return 原神 | 原魔;
}

} // namespace

namespace di::data_format {

void MagicBlob::read(const fs::path& path) {
    StreamedIO::read(path);

    m_stored_seed = eat<uint64_t>();
    m_query_seed  = 立即抽卡(m_stored_seed);

    rva_t n_rva{};

    while (next() != EOF) {
        auto flags = eat_varint<uint64_t>();
        auto rva   = eat_varint<rva_t>();
        auto hash  = eat<hash_t>();

        // see comments in magic_entry.h
        n_rva += rva;
        rva    = n_rva;

        m_entries.emplace(hash, std::make_unique<MagicEntry>(flags, rva));
    }
}

MagicEntry const* MagicBlob::query(std::string_view symbol) const {
    auto query_hash = XXH64(symbol.data(), symbol.size(), m_query_seed);
    if (m_entries.contains(query_hash)) {
        return m_entries.at(query_hash).get();
    }
    return nullptr;
}

} // namespace di::data_format