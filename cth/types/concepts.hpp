#pragma once
#include "utility.hpp"

#include <concepts>


#define CPT(concept) []<concept>{}


namespace cth::type {
template<class T, auto TCpt>
concept satisfies = requires {
    TCpt.template operator()<T>();
};

template<auto TCpt, class... Ts>
concept all_satisfy = (satisfies<Ts, TCpt> and ...);

template<auto TCpt, class... Ts>
concept any_satisfy = (satisfies<Ts, TCpt> or ...);

}


//independent concepts

namespace cth::type {


template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<class T>
concept character = std::same_as<pure_t<T>, char> or std::same_as<pure_t<T>, wchar_t>;

template<class T>
concept n_character = std::same_as<pure_t<T>, char>;

template<class T>
concept w_character = std::same_as<pure_t<T>, wchar_t>;




template<class T, auto Fn, class... Args>
concept callable = requires(T t) {
    std::invoke(Fn, t, std::declval<Args>()...);
};

template<class T>
concept has_release = callable<T, &T::release>;
template<class T>
concept has_get = callable<T, &T::get>;
template<class T>
concept has_deref = callable<T, &T::operator*>;



}
