#pragma once
#include "../macro.hpp"
#include "tuple.hpp"

#include <mdspan>


namespace cth::mta {

/**
 * @brief creates a std::extents with D dimensions and equal extents
 * @tparam D dimensions
 * @tparam Extent extent of every dimension
 * @return std::extents
 */
template<size_t D, auto Extent>
[[nodiscard]] cval auto mdextent() {
    return std::apply(
        []<class... Args>(Args... args) {
            return std::extents<decltype(Extent), (mta::zero<Args>(), Extent)...>{args...};
        },
        mta::n_tuple<D>(Extent)
    );
}


/**
 * @brief std::extents with D dimensions and equal extents
 * @tparam D dimensions
 * @tparam Extent extent of
 */
template<size_t D, auto Extent>
using sq_mdextent_t = decltype(mdextent<D, Extent>());

/**
 * @brief is a mdspan with base T and D dimensions
 * @tparam T base type
 * @tparam D dimensions
 * @tparam Extent extent of every dimension
 */
template<class T, size_t D, size_t Extent = std::dynamic_extent>
using sq_mdspan_t = std::mdspan<T, sq_mdextent_t<D, Extent>>;

template<class T, size_t... Extents>
using mdspan_t = std::mdspan<T, std::extents<size_t, Extents...>>;

}
