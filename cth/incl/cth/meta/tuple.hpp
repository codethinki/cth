#pragma once
#include "cth/macro.hpp"
#include "utility.hpp"
#include "cth/types.hpp"

#include <cstddef>
#include <utility>

namespace cth::mta {
/**
 * Mimics `std::apply` for type traits, i.e. applies a tuple pack as variadic type list
 * @tparam Trait to apply to
 * @tparam Tuple type list
 */
template<template<class...> class Trait, class Tuple>
struct trait_apply;

template<template<class...> class Trait, class... Ts>
struct trait_apply<Trait, std::tuple<Ts...>> : std::type_identity<Trait<Ts...>> {};

/**
 * shorthand for `trait_apply`
 * @tparam Trait to apply to
 * @tparam Tuple type list
 */
template<template<class...> class Trait, class Tuple>
using trait_apply_t = trait_apply<Trait, Tuple>::type;
}

namespace cth::mta {

namespace dev {
    template<class T, uz... I>
    consteval auto n_tuple_gen(T fill, std::index_sequence<I...>) { return std::tuple{(mta::zero(I), fill)...}; }
}

template<uz N, class T>
[[nodiscard]] consteval auto n_tuple(T fill = {}) { return dev::n_tuple_gen(fill, std::make_index_sequence<N>()); }

template<uz N>
[[nodiscard]] consteval auto n_tuple() { return n_tuple<N>(0); }

}
