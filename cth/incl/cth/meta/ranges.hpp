#pragma once
#include "concepts.hpp"
#include "cth/macro.hpp"
#include "traits.hpp"
#include "utility.hpp"

#include <cstddef>
#include <ranges>


// md_range

namespace cth::mta {

/**
 * counts the dimensions of `Rng`
 * @tparam Rng range
 * @param max where to stop counting (`MAX_DEPTH` => no limit)
 * @return dimensions of `Rng`, at most `max`
 */
template<class Rng>
consteval size_t dimensions(size_t max = MAX_DEPTH);

/**
 * `true` if `Rng` has at least `D` dimensions
 * @tparam Rng range
 * @tparam D dimensions
 */
template<class Rng, size_t D>
concept md_range = dimensions<Rng>(D) == D;


namespace dev {
    template<class Rng, size_t D>
    consteval auto md_range_value();
}


/**
 * value type of `Rng` after `D` dimensions
 * @details `D` above the dimensions of `Rng` -> the first non range type
 * @tparam Rng range
 * @tparam D dimensions to descend (`MAX_DEPTH` => all dimensions)
 */
template<class Rng, size_t D = MAX_DEPTH>
using md_range_value_t = type_of_t<dev::md_range_value<Rng, D>()>;

/**
 * `md_range_value_t` for `D = 2`
 */
template<class Rng>
using range2d_value_t = md_range_value_t<Rng, 2>;


template<class Rng, class T, size_t D = MAX_DEPTH>
concept md_range_over = std::same_as<T, md_range_value_t<Rng, D>>;

template<class Rng, auto TCpt, size_t D = MAX_DEPTH>
concept md_range_over_cpt = satisfies<md_range_value_t<Rng, D>, TCpt>;


template<class Rng, auto TCpt>
concept range_over_cpt = md_range_over_cpt<Rng, TCpt, 1>;

template<class Rng, class T>
concept range2d_over = md_range_over<Rng, T, 2>;
template<class Rng, auto TCpt>
concept range2d_over_cpt = md_range_over_cpt<Rng, TCpt, 2>;

template<class Rng, class T>
concept range_over = std::ranges::range<Rng> && std::same_as<std::ranges::range_value_t<Rng>, T>;

template<class Rng, class T>
concept range_over_convertible_to = std::ranges::range<Rng>
    && std::convertible_to<std::ranges::range_value_t<Rng>, T>;

}


namespace cth::rng {

template<class Rng>
concept static_dim_rng =
    std::ranges::range<Rng> && !std::same_as<std::ranges::range_value_t<Rng>, mta::rcvr_t<Rng>>;

/**
 * `true` if `Rng` is a viewable range or a copy of it is one
 */
template<class Rng>
concept viewable_rng = std::ranges::viewable_range<Rng> ||
    (std::ranges::viewable_range<mta::rcvr_t<Rng>> && requires(Rng rng) { mta::rcvr_t<Rng>{rng}; });

template<viewable_rng Rng>
constexpr auto to_viewable(Rng&& rng) {
    if constexpr(std::ranges::viewable_range<Rng>)
        return std::forward<Rng>(rng);
    else {
        static_assert(requires() { mta::rcvr_t<Rng>{rng}; }, "range must be view- or copyable");
        return mta::rcvr_t<Rng>{rng};
    }
}
}


#include "inl/ranges.inl"
