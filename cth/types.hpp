#pragma once
#include "type_traits.hpp"
#include <mdspan>


namespace cth {

namespace dev {
    template<class T, size_t... I>
    consteval auto n_tuple_gen(T, std::index_sequence<I...>) {
        using Tuple = decltype(std::tuple{std::declval<T>(), (std::declval<I>(), ...)});

        return std::type_identity<Tuple>{};
    }
}


template<class T, size_t N>
using n_tuple = typename decltype(dev::n_tuple_gen(std::declval<T>(), std::declval<std::make_index_sequence<N>>()))::type;


namespace dev {

    template<template<class...> class T, std::tuple>
    decltype(auto) applyType() {
        
    }

    template<size_t T, class Tuple, size_t Extent = std::dynamic_extent>
    consteval auto mdspan_gen() {
        using span = decltype(std::apply([](auto... extents) {
            using T = decltype()
        }));
    }
}

template<class T, size_t N>
using mdspan = std::mdspan<T, >;




}
