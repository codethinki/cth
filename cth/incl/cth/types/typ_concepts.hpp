#pragma once
#include "typ_utilty.hpp"
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

template<class T, auto TCpt>
concept negate = not satisfies<T, TCpt>;

template<class T>
concept w_character = std::same_as<pure_t<T>, wchar_t>;

template<class Obj, auto Fn, class... Args>
concept member_callable_with = requires(Obj obj) {
    std::invoke(Fn, obj, std::declval<Args>()...);
};

template<auto F, class... Args>
concept callable_with = requires {
    std::invoke(F, std::declval<Args>()...);
};
template<auto F, class... Args>
concept nothrow_callable_with = callable_with<F, Args...>
    and noexcept(std::invoke(F, std::declval<Args>()...));

template<auto F, class R, class... Args>
concept call_signature = requires {
    { std::invoke(F, std::declval<Args>()...) } -> std::same_as<R>;
};

template<auto F, class R, class... Args>
concept nothrow_call_signature = call_signature<F, R, Args...> and nothrow_callable_with<F, Args...>;


template<class T>
concept has_release = member_callable_with<T, &T::release>;
template<class T>
concept has_get = member_callable_with<T, &T::get>;
template<class T>
concept has_deref = requires(T t) {
    *t;
};

template<class T>
concept aggregate = std::is_aggregate_v<T>;

template<class T>
concept decayed = std::same_as<std::decay_t<T>, T>;

template<class T>
concept trivial = std::is_trivial_v<T>;
}

namespace cth::type {}
