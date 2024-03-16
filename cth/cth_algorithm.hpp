#pragma once
#include <algorithm>

#include "cth_concepts.hpp"
#include "cth_log.hpp"



namespace cth::algorithm::hash {
// from: https://stackoverflow.com/a/57595105
template<typename T, typename... Rest>
void combine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (cth::algorithm::hash::combine(seed, rest), ...);
}

}
