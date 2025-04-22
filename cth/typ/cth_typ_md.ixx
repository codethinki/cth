module;
#include "../macro.hpp"

export module cth.typ.md;

import std;
import cth.typ.tuple;
import cth.typ.utility;

export namespace cth::type {

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
            return std::extents<decltype(Extent), (type::zero<Args>(), Extent)...>{args...};
        },
        type::n_tuple<D>(Extent));
}


/**
 * @brief std::extents with D dimensions and equal extents
 * @tparam D dimensions
 * @tparam Extent extent of
 */
template<size_t D, auto Extent>
using mdextent_t = decltype(mdextent<D, Extent>());

/**
 * @brief is a mdspan with base T and D dimensions
 * @tparam T base type
 * @tparam D dimensions
 * @tparam Extent extent of every dimension
 */
template<class T, size_t D, size_t Extent = std::dynamic_extent>
using mdspan_t = std::mdspan<T, mdextent_t<D, Extent>>;


}
