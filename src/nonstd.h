#pragma once

template <typename To, typename From>
constexpr auto static_unique_ptr_cast(std::unique_ptr<From>&& F) {
    return std::unique_ptr<To>(static_cast<To*>(F.release()));
}

template <typename T>
concept Enumerate = std::is_enum_v<T>;

template <Enumerate T>
constexpr auto underlying_value(T v) {
    return static_cast<std::underlying_type_t<decltype(v)>>(v);
}

#if __cpp_constexpr >= 202211L // complete c++23 constexpr
#define DI_CONSTEXPR constexpr
#else
#define DI_CONSTEXPR inline
#endif

// a clang bug
#if __clang_major__ < 19
#undef DI_CONSTEXPR
#define DI_CONSTEXPR inline
#endif

// From:
// https://stackoverflow.com/questions/7110301/generic-hash-for-tuples-in-unordered-map-unordered-set

namespace std {

namespace {

// Code from boost
// Reciprocal of the golden ratio helps spread entropy
//     and handles duplicates.
// See Mike Seymour in magic-numbers-in-boosthash-combine:
//     http://stackoverflow.com/questions/4948780

template <class T>
constexpr void hash_combine(std::size_t& seed, T const& v) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// Recursive template code derived from Matthieu M.
template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
struct HashValueImpl {
    static void apply(size_t& seed, Tuple const& tuple) {
        HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
        hash_combine(seed, std::get<Index>(tuple));
    }
};

template <class Tuple>
struct HashValueImpl<Tuple, 0> {
    static void apply(size_t& seed, Tuple const& tuple) {
        hash_combine(seed, std::get<0>(tuple));
    }
};

} // namespace

template <typename... TT>
struct hash<std::tuple<TT...>> {
    constexpr size_t operator()(std::tuple<TT...> const& tt) const {
        size_t seed = 0;
        HashValueImpl<std::tuple<TT...>>::apply(seed, tt);
        return seed;
    }
};

} // namespace std
