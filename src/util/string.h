#pragma once

namespace di::util::string {

constexpr unsigned int H(std::string_view str, unsigned int hash = 0) {
    for (char c : str) hash = hash * 31 + static_cast<unsigned int>(c);
    return hash;
}

} // namespace di::util::string
