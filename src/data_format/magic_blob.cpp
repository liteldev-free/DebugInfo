#include "magic_blob.h"

#define XXH_INLINE_ALL
#include "xxhash.h"

#ifndef HIDWORD
#define HIDWORD(x) (*((int32_t*)&(x) + 1))
#endif

// copy from ida F5.
constexpr uint64_t unk_hash(uint64_t a1) {
    unsigned int v1; // eax
    int          v2; // edx
    int64_t      v3; // rdx

    v1 = ((33
               * ((4097 * HIDWORD(a1) + 2127912214)
                  ^ ((unsigned int)(4097 * HIDWORD(a1) + 2127912214) >> 19)
                  ^ 0xC761C23C)
           + 374761393)
          << 9)
       ^ (33
              * ((4097 * HIDWORD(a1) + 2127912214)
                 ^ ((unsigned int)(4097 * HIDWORD(a1) + 2127912214) >> 19)
                 ^ 0xC761C23C)
          - 369570787);
    v2 = 33
       * ((4097 * a1 + 2127912214)
          ^ ((unsigned int)(4097 * a1 + 2127912214) >> 19) ^ 0xC761C23C);
    v3 = (((v2 + 374761393) << 9) ^ (v2 - 369570787))
       + 8 * (((v2 + 374761393) << 9) ^ (unsigned int)(v2 - 369570787))
       - 42973499;
    return (v3 ^ (((unsigned int)v3 ^ 0xB55A4F090000uLL) >> 16))
         | ((((v1 + 8 * v1 - 42973499) & 0xFFFF0000)
             ^ (((v1 + 8 * v1 - 42973499) ^ 0xFFFFFFFFB55A4F09uLL) << 16))
            << 16);
}

namespace di::data_format {

void MagicBlob::read(const fs::path& path) {
    StreamedIO::read(path);

    m_stored_seed = eat<uint64_t>();
    m_query_seed  = unk_hash(m_stored_seed);

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