module;
#include "cthm/macro/utility.hpp"

#include <ranges>

export module cth.meta.ranges:inl;
import :decl;
import cth.meta.traits;


export namespace cth::mta::dev {
template<std::ranges::range Rng, size_t Max>
    cval size_t dimensions(size_t n) {
    if(n >= Max) return n;
    return dimensions<std::ranges::range_value_t<Rng>, Max>(n + 1);
}


template<class Rng, size_t Max>
[[nodiscard]] cval size_t dimensions() {
    return cpt_count<Rng, CPT(std::ranges::range), std::ranges::range_value_t, Max>();
}


template<class Rng, size_t D>
cval auto md_range_value() {
    static constexpr auto RANK = mta::dimensions<Rng, D>();

    using T = iterate_trait_t<Rng, std::ranges::range_value_t, RANK>;

    return std::type_identity<T>{};
}
}

