#pragma once

template <typename To, typename From>
constexpr auto static_unique_ptr_cast(std::unique_ptr<From>&& F) {
    return std::unique_ptr<To>(static_cast<To*>(F.release()));
}
