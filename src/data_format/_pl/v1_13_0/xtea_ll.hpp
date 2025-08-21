#pragma once

// An xtea variant algorithm.
// IDX = 11, L247 ~ L400

namespace di::data_format::_pl::v1_13_0 {

class XTeaLL {
public:
    struct Block {
        std::array<uint32_t, 8> parts;
    };
    static_assert(sizeof(Block) == 32);

    explicit XTeaLL(const Block& sample) : m_sample(sample) {};

    void decrypt(Block& data) {
        transform_block_a_v(data.parts[0], data.parts[1]);
        transform_block_b_v(data.parts[2], data.parts[3]);
        transform_block_b_i(data.parts[4], data.parts[5]);
        transform_block_a_i(data.parts[6], data.parts[7]);
    }

private:
    using u32 = uint32_t;
    using u16 = uint16_t;
    using u8  = uint8_t;

    // ----------------------------------------

    const static u32 delta = 0x9E3779B9;

    const static u32 mask_1 = 0x0000BEEF;
    const static u32 mask_2 = 0xFFFF4110;
    const static u32 mask_3 = 0x001FFFFC;
    const static u32 mask_4 = 0xFFFFFFFC;

    // ----------------------------------------

    Block const& m_sample;

    // ----------------------------------------

    constexpr auto mix32(u32 x) { return x + ((x >> 5) ^ (x << 4)); };

    constexpr auto beef(u32 a, u32 b, u32 c, u32 sum) {
        return (((u16)a + (u16)b - (u16)c) & mask_1) | (sum & mask_2);
    };

    constexpr auto idx_a(u32 sum1) {
        u32 t = (sum1 >> 11);
        return t & (t ^ mask_3);
    };

    template <bool Inverse>
    constexpr auto idx_b(u32 k, u32 s) {
        if (!Inverse) return ~((u8)k + (u8)s) & 3u;
        u32 t = k + s;
        return t & (t ^ mask_4);
    };

    template <bool Inverse>
    constexpr auto idx_c(u32 sum1) {
        if constexpr (!Inverse) return ((sum1 >> 9) & 0xC) / 4;
        return idx_a(sum1);
    };

    template <bool Inverse>
    constexpr auto idx_d(u32 vsum2, u32 vrsum2) {
        if constexpr (!Inverse) return ~(u8)vrsum2 & 3u;
        return vsum2 & (vsum2 ^ mask_4);
    };

    // ----------------------------------------

    template <bool Inverse>
    constexpr void transform_block_a(u32& v1, u32& v2) {
        u32 sum1  = delta * 64;
        u32 sum2  = delta * 63;
        u32 rsum1 = ~sum1;
        u32 rsum2 = ~sum2;

        for (int i = 0; i < 64; i++) {
            u32 s1 = m_sample.parts.at(idx_c<Inverse>(sum1));
            u32 s2 = m_sample.parts.at(idx_d<Inverse>(sum2, rsum2));
            if constexpr (!Inverse) {
                v2 -= mix32(v1) ^ beef(0, rsum1, s1, sum1 + s1) ^ mask_1;
                v1 -= (sum2 + s2) ^ (mix32(v2) & ((sum2 + s2) ^ (rsum2 - s2)));
            } else {
                v2 -= (sum1 + s1) ^ (mix32(v1) & ((sum1 + s1) ^ (rsum1 - s1)));
                v1 -= mix32(v2) ^ beef(0, rsum2, s2, sum2 + s2) ^ mask_1;
            }
            sum1  -= delta;
            sum2  -= delta;
            rsum1 += delta;
            rsum2 += delta;
        }
    };

    template <bool Inverse>
    constexpr void transform_block_b(u32& v1, u32& v2) {
        const u32 md1  = delta * 64;
        const u32 md2  = delta * 63;
        const u32 rmd1 = ~md1;
        const u32 rmd2 = ~md2;

        u32 sum1 = 0;
        u32 sum2 = 0;

        for (int i = 0; i < 64; i++) {
            u32 c1  = md1 + sum2;
            u32 s1  = m_sample.parts.at(idx_a(c1));
            v2     -= mix32(v1) ^ beef(sum1, rmd1, s1, s1 + c1) ^ mask_1;
            if constexpr (!Inverse) {
                u32 s2 = m_sample.parts.at(idx_b<Inverse>(rmd2, sum1));
                v1 -=
                    mix32(v2) ^ beef(rmd2, sum1, s2, s2 + sum2 + md2) ^ mask_1;
            } else {
                u32 s2  = m_sample.parts.at(idx_b<Inverse>(md2, sum2));
                u32 c2  = s2 + md2 + sum2;
                v1     -= c2 ^ (mix32(v2) & (c2 ^ (sum1 + rmd2 - s2)));
            }
            sum1 += delta;
            sum2 -= delta;
        }
    };

    constexpr void transform_block_a_v(u32& v1, u32& v2) {
        return transform_block_a<false>(v1, v2);
    }

    constexpr void transform_block_a_i(u32& v1, u32& v2) {
        return transform_block_a<true>(v1, v2);
    }

    constexpr void transform_block_b_v(u32& v1, u32& v2) {
        return transform_block_b<false>(v1, v2);
    }

    constexpr void transform_block_b_i(u32& v1, u32& v2) {
        return transform_block_b<true>(v1, v2);
    }
};

} // namespace di::data_format::_pl::v1_13_0
