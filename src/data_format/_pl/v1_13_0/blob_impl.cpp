#include "blob_impl.h"

#include "xtea_ll.hpp"

#include "third-party/chacha20.hpp"
#include "util/aes.h"

namespace {

// clang-format off

constexpr auto BLOB_MAGIC = 0x534442666F0B0E0BuLL;
constexpr auto BLOB_HEADER_SIZE = 0x14;

constexpr std::array<uint8_t, 16> CHACHA20_MAGIC = { // lldev's ninjutsu.
    0xE5, 0x93, 0x8E, 0xE6,
    0x88, 0x91, 0xE8, 0x8D,
    0x89, 0x52, 0x42, 0x4E,
    0xE6, 0x80, 0x8E, 0xE4,
};

constexpr std::array<uint8_t, 32> CHACHA20_KEY = {
    0xE5, 0x90, 0x93, 0xE6,
    0x88, 0x91, 0xE4, 0xB8,
    0x80, 0xE8, 0xB7, 0xB3,
    0xE9, 0x87, 0x8A, 0xE6,
    0x94, 0xBE, 0xE5, 0xBF,
    0x8D, 0xE6, 0x9C, 0xAF,
    0x61, 0x77, 0x63, 0x7A,
    0x72, 0x62, 0x6E, 0x00,
};

constexpr std::array<uint32_t, 4> DATAKEY_PART1_PRECURSOR = {
    0xE79E8CE7,
    0xADE5818C,
    0x9E9BE590,
    0x0082ADE9,
};

constexpr std::array<uint8_t, 32> DATAKEY_PART2_PRECURSOR_1 = {
    0x9C, 0x99, 0x18, 0xB8, 0x74, 0xF1, 0xF8, 0x6B,
    0x25, 0xAE, 0x87, 0xCE, 0x61, 0x32, 0xA0, 0x0C,
    0x01, 0xF3, 0x3A, 0x81, 0x5A, 0x16, 0x06, 0x4E,
    0x93, 0x16, 0xBE, 0xE8, 0xE3, 0xCA, 0x6F, 0xDF,
};

constexpr std::array<uint8_t, 32> DATAKEY_PART2_PRECURSOR_2 = {
    0x45, 0xB6, 0x4F, 0xDC, 0xAA, 0x0B, 0x4C, 0x67,
    0x11, 0x91, 0x3B, 0x75, 0x91, 0x83, 0x98, 0x05,
    0x77, 0x51, 0x5F, 0xD2, 0x09, 0x37, 0x1B, 0x36,
    0x0A, 0x77, 0x9B, 0x5E, 0xD6, 0xD9, 0xEE, 0x42,
};

// clang-format on

} // namespace

/*
    Magic Blob File Format
      v1.13.0
    ---------------------------------------  0x0
    | (u64) Magic Number                  |
    ---------------------------------------  0x8
    | (u32) UNK                           |
    ---------------------------------------  0xC
    | (u64) ChaCha20 Nonce                |
    ---------------------------------------  0x14
    | ***** ChaCha20 Encrypted Data ***** |
    ---------------------------------------  EOF

    ChaCha20 Decrypted Data
    ---------------------------------------  0x14 (before file header)
    | (u64) UNK                           |
    ---------------------------------------  0x1c
    | (u64) AES Encrypted Data Part 1 Len |
    ---------------------------------------  0x24
    | (b16) Stored Key Lower 16 Bytes     |
    ---------------------------------------  0x34
    | **** AES Encrypted Data Part 1 **** |
    ---------------------------------------  0x34 + enc_len_p1
    | (b16) Stored Key Higher 16 Bytes    |
    ---------------------------------------  0x34 + enc_len_p1 + 0x10
    | (u64) AES Encrypted Data Part 2 Len |
    ---------------------------------------  0x34 + enc_len_p1 + 0x18
    | **** AES Encrypted Data Part 2 **** |
    ---------------------------------------  EOF

*/

namespace di::data_format::_pl::v1_13_0 {

void MagicBlobImpl::read(const fs::path& path) {
    StreamedIO::read(path);

    // header

    auto magic          = get<uint64_t>();
    auto unk_1          = get<uint32_t>();
    auto chacha20_nonce = get<uint64_t>();

    if (magic != BLOB_MAGIC) {
        std::println("error!");
        return;
    }

    if (unk_1 != 1) {
        std::println("error!");
        return;
    }

    // perform chacha20 decryption

    Chacha20 chacha20(
        CHACHA20_MAGIC.data(),
        CHACHA20_KEY.data(),
        (uint8_t*)&chacha20_nonce,
        0
    );

    chacha20.decrypt(
        reinterpret_cast<uint8_t*>(underlying().data()) + BLOB_HEADER_SIZE,
        size() - BLOB_HEADER_SIZE
    );

    // chacha20 decrypted

    XTeaLL::Block data_key;

    auto unk_2              = get<uint64_t>(); // may be signed, L241
    auto enc_data_part1_len = get<uint64_t>();
    data_key.parts[0]       = get<uint32_t>(); // L231
    data_key.parts[1]       = get<uint32_t>();
    data_key.parts[2]       = get<uint32_t>();
    data_key.parts[3]       = get<uint32_t>();
    seek(position() + enc_data_part1_len);
    data_key.parts[4]       = get<uint32_t>(); // L244
    data_key.parts[5]       = get<uint32_t>();
    data_key.parts[6]       = get<uint32_t>();
    data_key.parts[7]       = get<uint32_t>();
    auto enc_data_part2_len = get<uint64_t>();

    // perform aes decryption

    auto enc_data_part1_begin = underlying().data() + 0x34;
    auto enc_data_part2_begin =
        underlying().data() + 0x34 + enc_data_part1_len + 0x18;

    XTeaLL({
               DATAKEY_PART1_PRECURSOR[0],
               DATAKEY_PART1_PRECURSOR[1],
               DATAKEY_PART1_PRECURSOR[2],
               DATAKEY_PART1_PRECURSOR[3],
           })
        .decrypt(data_key);

    std::array<uint8_t, 32> aes_key_part1;
    std::memcpy(aes_key_part1.data(), (char*)&data_key.parts, 32);

    std::array<uint8_t, 32> aes_key_part2;
    for (int i = 0; i < 32; i++) {
        aes_key_part2[i] = DATAKEY_PART2_PRECURSOR_1[i]
                         ^ DATAKEY_PART2_PRECURSOR_2[i] ^ aes_key_part1[i];
    }

    auto data_part1 = AES(aes_key_part1)
                          .decrypt(
                              {reinterpret_cast<uint8_t*>(enc_data_part1_begin),
                               enc_data_part1_len}
                          );

    auto data_part2 = AES(aes_key_part2)
                          .decrypt(
                              {reinterpret_cast<uint8_t*>(enc_data_part2_begin),
                               enc_data_part2_len}
                          );
}

} // namespace di::data_format::_pl::v1_13_0
