#pragma once
#include "cth/meta/traits.hpp"


namespace cth::mta {

template<class Rng>
[[nodiscard]] cval size_t dimensions(size_t max) {
    return trait_count<Rng, std::ranges::range_value_t>(max);
}


namespace dev {
    template<class Rng, size_t D>
    cval auto md_range_value() {
        static constexpr auto RANK = mta::dimensions<Rng>(D);

        return repeat_trait<RANK, std::ranges::range_value_t, Rng>{};
    }
} // namespace dev


}
