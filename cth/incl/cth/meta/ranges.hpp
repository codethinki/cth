#pragma once
#include "concepts.hpp"
#include "traits.hpp"
#include "utility.hpp"
#include "cth/macro.hpp"

#include <ranges>


//md_range

namespace cth::mta {

/**
 * @brief counts the dimensions of @ref Rng
 * @tparam Rng range
 * @tparam Max optional max search depth
 */
template<class Rng, size_t Max = MAX_DEPTH>
cval size_t dimensions();

/**
 * @brief checks if @ref Rng is of at least @ref D dimensions
 * @tparam Rng range
 * @tparam D dimensions
 */
template<class Rng, size_t D>
concept md_range = dimensions<Rng, D>() == D;


namespace dev {
    template<class Rng, size_t D>
    cval auto md_range_value();
}


/**
 * @brief type trait to get the first non range type or the @ref D -th dimension range value type of @ref Rng
 * @tparam Rng range
 * @tparam D dimension (optional)
 */
template<class Rng, size_t D = MAX_DEPTH>
using md_range_value_t = typename decltype(dev::md_range_value<Rng, D>())::type;

/**
 * @brief shortcut to @ref md_range_value_t<Rng, 2>
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

}

namespace cth::rng {

template<class Rng>
concept static_dim_rng = std::ranges::range<Rng> && !std::same_as<std::ranges::range_value_t<Rng>, mta::rcvr_t<Rng>>;

/**
 * @brief accepts viewable or copy viewable ranges
 */
template<class Rng>
concept viewable_rng = std::ranges::viewable_range<Rng>
    || (
        std::ranges::viewable_range<mta::rcvr_t<Rng>>
        && requires(Rng rng) { mta::rcvr_t<Rng>{rng}; }
    );

template<viewable_rng Rng>
auto cxpr to_viewable(Rng&& rng) {
    if constexpr(std::ranges::viewable_range<Rng>) return std::forward<Rng>(rng);
    else {
        static_assert(requires() { mta::rcvr_t<Rng>{rng}; }, "range must be view- or copyable");
        return mta::rcvr_t<Rng>{rng};
    }
}
}

#include "inl/ranges.inl"
