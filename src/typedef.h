#pragma once

namespace di {

using rva_t  = uint32_t;
using addr_t = uint64_t;

using hash_t = uint64_t;

} // namespace di

namespace fs = std::filesystem;

// shit!
template <typename T>
struct TypeOnly {};
