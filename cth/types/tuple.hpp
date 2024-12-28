#pragma once
#include "utility.hpp"

#include <utility>


//tuple functions

namespace cth::type {

namespace dev {
    template<class T, size_t... I>
    consteval auto n_tuple_gen(T fill, std::index_sequence<I...>) { return std::tuple{(type::zero(I), fill)...}; }
}

template<size_t N, class T>
consteval auto n_tuple(T fill = {}) {
    return dev::n_tuple_gen(fill, std::make_index_sequence<N>());
}

template<size_t N>
consteval auto n_tuple() { return n_tuple<N>(0); }

}