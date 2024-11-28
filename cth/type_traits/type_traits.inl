#pragma once

#include<concepts>
#include <stdexcept>
#include <type_traits>

//trait functions
namespace cth::type {
template<class T, template<class> class Trait, size_t N>
consteval auto apply_trait() {
    constexpr bool valid = applicable<T, Trait>;

    if constexpr(N == 0) return std::type_identity<T>{};
    else if constexpr(!valid) static_assert(valid, "[Trait] not applicable on [T]");
    else return apply_trait<Trait<T>, Trait, N - 1>();
}

namespace dev {
    template<class T, template<class> class Trait, size_t Max>
    consteval size_t trait_count(size_t n = 0) {
        if constexpr(!applicable<T, Trait>) return n;
        else {
            if(n >= Max) return n;
            return trait_count<apply_trait_t<T, Trait>, Trait, Max>(n + 1);
        }
    }
}


template<class T, template<class> class Trait, size_t Max>
consteval size_t trait_count() { return dev::trait_count<T, Trait, Max>(); }

namespace dev {
    template<class T, auto TCpt, template<class> class Trait, size_t MaxDepth>
    consteval size_t cpt_count(size_t n = 0) {
        if(n >= MaxDepth) return n;
        if constexpr(!satisfies<T, TCpt>) return n;
        else if constexpr(!applicable<T, Trait>) throw std::logic_error{"[Trait] must be applicable on [T], (TCpt satisfied and MaxDepth not reached)"};
        else {
            using U = apply_trait_t<T, Trait>;
            return cpt_count<U, TCpt, Trait, MaxDepth>(n + 1);
        }
    }
}

template<class T, auto TCpt, template<class> class Trait, size_t MaxDepth>
consteval size_t cpt_count() { return dev::cpt_count<T, TCpt, Trait, MaxDepth>(); }


}

//any_of

namespace cth::type {
/**
 * \brief ::type is equal to first of Ts... that's equal to T or Fallback if none are
 * \tparam Fallback if none of Ts... are equal to T
 */
template<typename Fallback, typename T, typename... Ts>
struct fallback_any_of {
    using type = std::conditional_t<is_any_of<T, Ts...>, T, Fallback>;
};
/**
 * \brief equal to first of Ts... that's equal to T or Fallback if none are
 * \tparam Fallback if none of Ts... are equal to T
 */
template<typename Fallback, typename T, typename... Ts>
using fallback_any_of_t = typename fallback_any_of<Fallback, T, Ts...>::type;

/**
 * \brief ::type is equal to first of Ts... that's equal to T
 */
template<typename T, typename... Ts>
struct any_of {
    using type = typename fallback_any_of<empty_t, T, Ts...>::type;
    static_assert(!std::same_as<type, empty_t>, "None of the types are the same as T");
};

template<typename T, typename... Ts> requires (is_any_of<T, Ts...>)
auto to_same_of(T&& arg) { return std::forward<T>(arg); }

template<typename... Ts, typename T>
auto to_same(T&& arg) { return type::to_same_of<T, Ts...>(std::forward<T>(arg)); }
}


//convert_to_any

namespace cth::type {
template<typename Fallback, typename T, typename... Ts>
struct fallback_convert_to_any_helper;
template<typename Fallback, typename T, typename First, typename... Rest>
struct fallback_convert_to_any_helper<Fallback, T, First, Rest...> {
    using type = std::conditional_t<std::convertible_to<T, First>, First, typename fallback_convert_to_any_helper<Fallback, T, Rest...>::type>;
};
template<typename Fallback, typename T>
struct fallback_convert_to_any_helper<Fallback, T> {
    using type = Fallback;
};
template<typename Fallback, typename T, typename... Ts>
using fallback_convert_to_any_of_helper_t = typename fallback_convert_to_any_helper<Fallback, T, Ts...>::type;

template<typename Fb, typename T, typename... Ts>
struct fallback_convert_to_any {
    using type = std::conditional_t<is_any_of<T, Ts...>,
        fallback_any_of_t<Fb, T, Ts...>,
        fallback_convert_to_any_of_helper_t<Fb, T, Ts...>
    >;
};

template<typename T, typename... Ts>
struct convert_to_any {
    using type = typename fallback_convert_to_any<empty_t, T, Ts...>::type;
    static_assert(!std::same_as<type, empty_t>, "None of the types are convertible to T");
};



template<typename U, typename... Ts, typename T> requires(convertible_to_any<T, Ts...>)
auto to_convertible_from(T&& arg) {
    if constexpr(is_any_of<T, Ts...>) return type::to_same_of<T, Ts...>(std::forward<T>(arg));
    else return static_cast<convert_to_any_t<U, Ts...>>(arg);
}

template<typename... Ts, typename T>
auto to_convertible(T&& arg) { return type::to_convertible_from<T, Ts...>(std::forward<T>(arg)); }
}

//construct_any_from

namespace cth::type {
template<typename Fallback, typename T, typename... Ts>
struct fallback_construct_any_from_helper;
template<typename Fallback, typename T, typename First, typename... Rest>
struct fallback_construct_any_from_helper<Fallback, T, First, Rest...> {
    using type = std::conditional_t<std::constructible_from<T, First>,
        First,
        typename fallback_construct_any_from_helper<Fallback, T, Rest...>::type
    >;
};
template<typename Fallback, typename T>
struct fallback_construct_any_from_helper<Fallback, T> {
    using type = Fallback;
};
template<typename Fallback, typename T, typename... Ts>
using fallback_construct_any_from_helper_t = typename fallback_construct_any_from_helper<Fallback, T, Ts...>::type;

template<typename Fallback, typename T, typename... Ts>
struct fallback_construct_any_from {
    using type = std::conditional_t<any_convertible_to<T, Ts...>,
        fallback_convert_to_any_t<Fallback, T, Ts...>,
        fallback_construct_any_from_helper_t<Fallback, T, Ts...>
    >;

};

/**
 * \brief equal to first of Ts... that's constructible from T or Fallback if none are 
 * \tparam Fallback if none of Ts... are constructible from T
 * \tparam T from
 * \tparam Ts to any of Ts...
 * \note prioritizes convert_any_from_t<T, Ts...> if available
 */
template<typename Fallback, typename T, typename... Ts>
using fallback_construct_any_from_t = typename fallback_construct_any_from<Fallback, T, Ts...>::type;


/**
 * \brief ::type is equal to first type of Ts... that is constructible from T
 * \tparam T from
 * \tparam Ts to any of Ts...
 * \note prioritizes convert_any_from_t<T, Ts...> if available
 */
template<typename T, typename... Ts>
struct construct_any_from {
    using type = fallback_construct_any_from_t<void, T, Ts...>;
    static_assert(!std::is_same_v<type, void>, "None of the types are constructable from T");
};

template<typename U, typename... Ts, typename T> requires(any_constructible_from<T, Ts...>)
auto to_constructible_from(T&& arg) {
    if constexpr(convertible_to_any<U, Ts...>) return cth::type::to_convertible_from<U, Ts...>(std::forward<T>(arg));
    else return construct_any_from_t<U, Ts...>{arg};
}
template<typename... Ts, typename T>
auto to_constructible(T&& arg) { return type::to_constructible_from<T, Ts...>(std::forward<T>(arg)); }

}

namespace cth::type {
namespace dev {
    template<std::ranges::range Rng, size_t Max>
    consteval size_t dimensions(size_t n) {
        if(n >= Max) return n;
        return dimensions<std::ranges::range_value_t<Rng>, Max>(n + 1);
    }
}

template<class Rng, size_t Max>
consteval size_t dimensions() { return dev::dimensions<Rng, Max>(0); }

template<class Rng, size_t D, class>
struct md_range_value {
    using type = Rng;
    static_assert(D == 0, "failed to substitute, D <= dimensions<Rng>() required");
};

template<class Rng, size_t D>
struct md_range_value<Rng, D, std::enable_if_t<(std::ranges::range<Rng> && D > 0),
        empty_t>> : md_range_value<std::ranges::range_value_t<Rng>, D != MAX_DEPTH ? D - 1 : dimensions<Rng>() - 1> {};
}
