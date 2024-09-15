#pragma once
#include<concepts>
#include <type_traits>

namespace cth::type {



template<class Rng, size_t D>
using md_range_val_t = std::void_t<md_range_val<Rng, D>;

template<class Rng>
using range_val_t = std::ranges::range_value_t<Rng>;

template<class Rng>
using range2d_val_t = md_range_val_t<Rng, 2>;

template<typename T, typename... Ts> struct is_any_of : bool_constant<std::disjunction_v<std::same_as<T, Ts>...>> {};


//
//any_of
//
template<class T, class... Ts> constexpr bool is_any_of_v;

template<typename T, typename... Ts> constexpr bool is_any_of_v = is_any_of<T, Ts...>::value;

/**
 * \brief ::type is equal to first of Ts... that's equal to T or Fallback if none are
 * \tparam Fallback if none of Ts... are equal to T
 */
template<typename Fallback, typename T, typename... Ts>
struct fallback_any_of {
    using type = std::conditional_t<is_any_of_v<T, Ts...>, T, Fallback>;
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


//
//convert_to_any
//

template<typename T, typename... Ts> struct is_any_convertible_to : bool_constant<std::disjunction_v<std::is_convertible<T, Ts>...>> {};
template<typename T, typename... Ts> constexpr bool is_any_convertible_to_v = (is_any_convertible_to<T, Ts...>::value);


template<typename Fallback, typename T, typename... Ts>
struct fallback_convert_to_any_helper;
template<typename Fallback, typename T, typename First, typename... Rest>
struct fallback_convert_to_any_helper<Fallback, T, First, Rest...> {
    using type = std::conditional_t<std::is_convertible_v<T, First>, First, typename fallback_convert_to_any_helper<Fallback, T, Rest...>::type>;
};
template<typename Fallback, typename T>
struct fallback_convert_to_any_helper<Fallback, T> {
    using type = Fallback;
};
template<typename Fallback, typename T, typename... Ts>
using fallback_convert_to_any_of_helper_t = typename fallback_convert_to_any_helper<Fallback, T, Ts...>::type;

template<typename Fallback, typename T, typename... Ts>
struct fallback_convert_to_any {
    using type = std::conditional_t<is_any_of_v<T, Ts...>,
        fallback_any_of_t<Fallback, T, Ts...>,
        fallback_convert_to_any_of_helper_t<Fallback, T, Ts...>
    >;
};

template<typename T, typename... Ts>
struct convert_to_any {
    using type = typename fallback_convert_to_any<empty_t, T, Ts...>::type;
    static_assert(!std::same_as<type, empty_t>, "None of the types are convertible to T");
};


//
//construct_any_from
//


template<typename T, typename... Ts> struct is_any_constructible_from : bool_constant<std::disjunction_v<std::is_constructible<Ts, T>...>> {};
template<typename T, typename... Ts> constexpr bool is_any_constructible_from_v = is_any_constructible_from<T, Ts...>::value;
template<typename T, typename... Ts>
concept any_constructible_from = is_any_constructible_from_v<T, Ts...>;

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
    using type = std::conditional_t<is_any_convertible_to_v<T, Ts...>,
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

/**
 * \brief equal to first type of Ts... that is constructible from T
 * \tparam T from
 * \tparam Ts to any of Ts...
 * \note prioritizes convert_any_from_t<T, Ts...> if available
 */
template<typename T, typename... Ts>
using construct_any_from_t = typename construct_any_from<T, Ts...>::type;

}


//--------------
//   functions
//--------------


namespace cth::type {

template<class T, size_t Max = 0> requires(!std::ranges::range<T>)
consteval size_t dimensions(size_t n = 0) { return n; }

template<std::ranges::range Rng, size_t Max = 0>
consteval size_t dimensions(size_t n = 0) {
    if constexpr(Max != 0 && n >= Max) return n;
    else return dimensions<std::ranges::range_value_t<Rng>, Max>(n + 1);
}


/**
 * \brief converts to the first type of Ts... that is the same as T
 * \tparam Ts to any of Ts...
 * \tparam T from
 */
template<typename T, typename... Ts> requires (is_any_of_v<T, Ts...>)
auto to_same_of(T&& arg) { return std::forward<T>(arg); }

/**
 * \brief converts to the first type of Ts... that is the same as T
 * \tparam Ts to any of Ts...
 * \tparam T from
 */
template<typename... Ts, typename T>
auto to_same(T&& arg) { return type::to_same_of<T, Ts...>(std::forward<T>(arg)); }

/**
 * \brief converts to the first type of Ts... that is convertible from T
 * \tparam Ts to any of Ts...
 * \tparam U from
 * \note prioritizes to_same_of<T, Ts...> if available
 */
template<typename U, typename... Ts, typename T> requires(is_any_convertible_to_v<T, Ts...>)
auto to_convertible_from(T&& arg) {
    if constexpr(is_any_of_v<T, Ts...>) return type::to_same_of<T, Ts...>(std::forward<T>(arg));
    else return static_cast<convert_to_any_t<U, Ts...>>(arg);
}
/**
 * \brief converts to the first type of Ts... that is convertible from T
 * \tparam Ts to any of Ts...
 * \tparam T from
 * \note prioritizes to_same_of<T, Ts...> if available
 */
template<typename... Ts, typename T>
auto to_convertible(T&& arg) { return type::to_convertible_from<T, Ts...>(std::forward<T>(arg)); }

/**
 * \brief constructs the first type of Ts... that is constructible from T
 * \tparam Ts to any of Ts...
 * \tparam U from
 * \note prioritizes to_convertible<T, Ts...> if available
 */
template<typename U, typename... Ts, typename T> requires(is_any_constructible_from_v<T, Ts...>)
auto to_constructible_from(T&& arg) {
    ;
    if constexpr(is_any_convertible_to_v<U, Ts...>) return cth::type::to_convertible_from<U, Ts...>(std::forward<T>(arg));
    else return construct_any_from_t<U, Ts...>{arg};
}
/**
 * \brief constructs the first type of Ts... that is constructible from T
 * \tparam Ts to any of Ts...
 * \tparam T from
 * \note prioritizes to_convertible<T, Ts...> if available
 */
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
}
