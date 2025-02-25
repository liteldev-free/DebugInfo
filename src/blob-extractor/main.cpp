#include <bitset>
#include <cstdint>
#include <fstream>
#include <print>
#include <unordered_map>

#define XXH_INLINE_ALL
#include "xxhash.h"

class File : public std::ifstream {
public:
    using std::ifstream::basic_ifstream;

    template <typename T>
    inline T read() {
        T value;
        std::ifstream::read((char*)&value, sizeof(T));
        return value;
    }

    template <std::unsigned_integral T>
    inline T read_varint() {
        T   res   = 0;
        int shift = 0;
        while (true) {
            auto byte  = std::ifstream::get();
            res       |= static_cast<T>(byte & 0x7F) << shift;
            if ((byte & 0x80) == 0) break;
            shift += 7;
        }
        return res;
    }
};

#define HIDWORD(x) (*((int32_t*)&(x) + 1))

uint64_t unk_hash(uint64_t a1) {
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

struct Entry {
    std::bitset<64> flags;
    uint32_t        rva;
    uint64_t        hash;

    constexpr bool is_function() { return flags[0]; }
    constexpr bool _unk2() { return flags[1]; }
    constexpr bool is_verbose() { return flags[2]; }
    constexpr bool _unk4() { return flags[3]; }
};

int main(int argc, char** argv) {
    File data("bedrock_runtime_data", std::ios::binary);

    uint32_t t_rva{};
    auto     record_seed = data.read<uint64_t>();
    auto     twin_seed   = unk_hash(record_seed);

    std::println("Record seed: {:#x}", record_seed);
    std::println("Twin seed: {:#x}", twin_seed);

    std::unordered_map<uint64_t, Entry> map;

    while (data.peek() != EOF) {
        Entry entry;
        entry.flags = data.read_varint<uint64_t>();
        entry.rva   = data.read_varint<uint32_t>();
        entry.hash  = data.read<uint64_t>();

        t_rva     += entry.rva;
        entry.rva  = t_rva;

        map.emplace(entry.hash, entry);
        // entry.print_debug_string();
    }

    std::string_view test_query_name = "main";
    auto             test_query_hash =
        XXH64(test_query_name.data(), test_query_name.size(), twin_seed);

    if (map.contains(test_query_hash)) {
        std::println("RVA of main(): {:#x}", map.at(test_query_hash).rva);
    } else {
        std::println("RVA of main(): INVALID.");
    }

    return 0;
}
