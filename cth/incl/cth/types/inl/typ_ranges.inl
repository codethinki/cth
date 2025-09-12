#pragma once
#include "../typ_traits.hpp"



namespace cth::type {
namespace dev {
    template<std::ranges::range Rng, size_t Max>
    cval size_t dimensions(size_t n) {
        if(n >= Max) return n;
        return dimensions<std::ranges::range_value_t<Rng>, Max>(n + 1);
    }
}


template<class Rng, size_t Max>
[[nodiscard]] cval size_t dimensions() { return cpt_count<Rng, CPT(std::ranges::range), std::ranges::range_value_t, Max>(); }


namespace dev {
    template<class Rng, size_t D>
    cval auto md_range_value() {
        static constexpr auto RANK = type::dimensions<Rng, D>();

        using T = apply_trait_t<Rng, std::ranges::range_value_t, RANK>;

        return std::type_identity<T>{};
    }
} //namespace dev



}
