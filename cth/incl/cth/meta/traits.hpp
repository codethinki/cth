#pragma once
#include "utility.hpp"

#include "cth/macro.hpp"

#include <tuple>
#include <type_traits>


// constant struct

namespace cth::mta {

template<class T, template<class> class Trait>
concept simple_trait_applicable = requires() { typename Trait<T>; };

template<template<class> class Trait, class... Args>
concept trait_applicable = requires() { typename Trait<Args...>; };
}


// trait functions

namespace cth::mta {
template<size_t N, template<class> class Trait, class T> requires(trait_applicable<Trait, T> || N == 0)
struct repeat_trait : repeat_trait<N - 1, Trait, Trait<T>> {};

template<template<class> class Trait, class T>
struct repeat_trait<0, Trait, T> : std::type_identity<T> {};

template<size_t N, template<class> class Trait, class T>
using repeat_trait_t = repeat_trait<N, Trait, T>::type;


template<class T, template<class> class Trait>
[[nodiscard]] cval size_t trait_count(size_t max_depth = MAX_DEPTH);

template<class T, auto TCpt, template<class> class Trait>
[[nodiscard]] cval size_t cpt_count(size_t max_depth = MAX_DEPTH);

/**
 * should not be used, use std::forward_like instead
 */
template<class From, class To>
struct fwd_const {
    using type = std::conditional_t<std::is_const_v<std::remove_reference_t<From>>, std::add_const_t<To>, To>;
};


/**
 * should not be used, use std::forward_like instead
 */
template<class From, class To>
using fwd_const_t = fwd_const<From, To>::type;

template<size_t I, class... Ts>
using get_t = std::tuple_element_t<I, std::tuple<Ts...>>;


}


namespace cth::mta {
template<class T>
using rcvr_t = std::remove_cvref_t<T>;
}


#include "inl/traits.inl"

#include "traits/trait_packs.hpp"


namespace cth::mta {

namespace dev {
    template<class T>
    struct remove_rvalue_reference {
        using value_type = T;
    };

    template<class T>
    struct remove_rvalue_reference<T&&> {
        using value_type = T;
    };
}


template<class T>
using remove_rvalue_reference_t = dev::remove_rvalue_reference<T>::value_type;

template<class T>
using remove_rvalue_ref_t = remove_rvalue_reference_t<T>;



}
