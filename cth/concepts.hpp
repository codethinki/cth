#pragma once
#include "type_traits.hpp"

#define cpt(concept) []<concept>{}


namespace cth::type {
template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<class T, auto TCpt>
concept satisfies = requires {
    TCpt.template operator()<T>();
};


template<class Rng, class T>
concept range_over = std::ranges::range<Rng> and std::same_as<std::ranges::range_value_t<Rng>, T>;

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

template<class T>
concept has_get = requires(T t) {
    t.get();
};

template<class T>
concept has_release = requires(T t) {
    t.release();
};

template<class T>
concept has_deref = requires(T t) {
    *t;
};



template<typename T>
concept character = std::_Is_any_of_v<std::remove_cv_t<T>, char, wchar_t>;


template<typename T>
concept string_view_convertable = is_string_view_convertable_v<T>;

template<typename T>
concept literal = is_string_view_convertable_v<T> || is_char_v<T>;

}
