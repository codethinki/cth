#pragma once
#include "utility.hpp"
#include "concepts.hpp"

#include <ranges>

#include "../utility.hpp"

//md_range

namespace cth::type {

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
concept range_over = std::ranges::range<Rng> and std::same_as<std::ranges::range_value_t<Rng>, T>;

}


#include "inl/ranges.inl"
