#pragma once
#include "type_traits.hpp"

#define cpt(concept) []<concept>{}


namespace cth::type {



template<class Rng, size_t N, class T>
concept md_range_over = type::is_md_range_v<Rng, N> and std::same_as<md_range_val_t<Rng, N>, T>;

template<class Rng, size_t N, auto TCpt>
concept md_range_over_cpt = type::is_md_range_v<Rng, N> and satisfies<md_range_val_t<Rng, N>, TCpt>;

template<class Rng, auto TCpt>
concept range_over_cpt = std::ranges::range<Rng> and satisfies<std::ranges::range_value_t<Rng>, TCpt>;


template<class Rng, class T>
concept range2d_over = std::ranges::range<Rng> and range_over<std::ranges::range_value_t<Rng>, T>;

template<class Rng, auto TCpt>
concept range2d_over_cpt = std::ranges::range<Rng> and range_over_cpt<std::ranges::range_value_t<Rng>, TCpt>;


template<class Rng, size_t N>
concept md_range = type::is_md_range_v<Rng, N>;




template<typename T, typename... Ts>
concept any_convertible_to = is_any_convertible_to_v<T, Ts...>;


template<typename T>
concept character = std::_Is_any_of_v<std::remove_cv_t<T>, char, wchar_t>;

}
