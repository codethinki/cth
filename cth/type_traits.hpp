#pragma once


#include <type_traits>

#include <string>
#define cpt(concept) []<concept>{}


//constant struct

namespace cth::type {

template<auto V>
struct constant {
    static constexpr auto VALUE = V;

    using value_type = decltype(VALUE);
    using type = constant;

    [[nodiscard]] constexpr operator value_type() const noexcept { return VALUE; }

    [[nodiscard]] constexpr value_type operator()() const noexcept { return VALUE; }
};

template<bool V>
using bool_constant = constant<V>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;
}


//independent constants

namespace cth::type {

constexpr auto no_range = [] {};
constexpr auto empty = [] {};


using no_range_t = decltype(no_range);
using empty_t = decltype(empty);

}


//independent usings

namespace cth::type {

template<typename T>
using pure_t = std::remove_cv_t<std::decay_t<T>>;

}


//independent concepts

namespace cth::type {
template<class T, auto TCpt>
concept satisfies = requires {
    TCpt.template operator()<T>();
};

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<class Rng, class T>
concept range_over = std::ranges::range<Rng> and std::same_as<std::ranges::range_value_t<Rng>, T>;

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

}


//typedefs

namespace cth::type {
template<class T>
using pure_t = std::remove_cv_t<std::decay_t<T>>;
}


//any... type traits

namespace cth::type {
template<class T, class... Ts> struct is_any_of;

/**
 * \brief ::type is equal to first of Ts... that's equal to T or Fallback if none are
 * \tparam Fb (Fallback) if none of Ts... are equal to T
 */
template<class Fb, class T, class... Ts> struct fallback_any_of;

/**
 * \brief ::type is equal to first of Ts... that's equal to T
 */
template<class T, class... Ts>
struct any_of;



/**
 * @brief shortcut to @ref any_of::type
 */
template<class T, class... Ts>
using any_of_t = typename any_of<T, Ts...>::type;

template<class T, class... Ts> struct is_any_convertible_to;

/**
 * @brief ::type is equal to first of Ts... that's convertible from T or Fallback if none are
 * @tparam Fb if none of Ts... are convertible from T
 * @tparam T from
 * @tparam Ts to any of
 * @note prioritizes any_of_t<T, Ts...> if available
 */
template<typename Fb, typename T, typename... Ts>
struct fallback_convert_to_any;

/**
 * @brief shortcut to @ref fallback_convert_to_any_t::type
 */
template<class Fb, class T, class... Ts>
using fallback_convert_to_any_t = typename fallback_convert_to_any<Fb, T, Ts...>::type;

/**
 * @brief shortcut to @ref fallback_any_of::type
 */
template<class Fb, class T, class... Ts>
using fallback_any_of_t = typename fallback_any_of<Fb, T, Ts...>::type;

/**
 * \brief ::type is equal to first of Ts... that's convertible from T
 * \tparam T from
 * \tparam Ts to any of Ts...
 * \note prioritizes any_of_t<T, Ts...> if available
 */
template<typename T, typename... Ts>
struct convert_to_any;


/**
 *@brief shortcut to @ref convert_to_any_t::type
 */
template<typename T, typename... Ts>
using convert_to_any_t = typename convert_to_any<T, Ts...>::type;

/**
 * \brief ::type is equal to first of Ts... that's constructible from T or Fallback if none are 
 * \tparam Fallback if none of Ts... are constructible from T
 * \tparam T from
 * \tparam Ts to any of Ts...
 * \note prioritizes convert_any_from_t<T, Ts...> if available
 */
template<typename Fallback, typename T, typename... Ts>
struct fallback_construct_any_from;

/**
 * \brief ::type is equal to first type of Ts... that is constructible from T
 * \tparam T from
 * \tparam Ts to any of Ts...
 * \note prioritizes convert_any_from_t<T, Ts...> if available
 */
template<typename T, typename... Ts>
struct construct_any_from;
}


//md_range

namespace cth::type {
namespace dev {
    template<class T, size_t Max = 0> requires(!std::ranges::range<T>)
    consteval size_t dimensions(size_t n) { return n; }
    template<std::ranges::range Rng, size_t Max = std::numeric_limits<size_t>::max()>
    consteval size_t dimensions(size_t n);
}


template<class Rng, size_t Max = std::numeric_limits<size_t>::max()>
consteval size_t dimensions();

template<class Rng, size_t N>
concept md_range = dimensions<Rng, N>() == N;

template<class Rng, size_t D, class = empty_t>
struct md_range_value {
    using type = Rng;
    static_assert(md_range<Rng, D>, "failed to substitute, D <= dimensions<Rng>() required");
};

template<class Rng, size_t N>
struct md_range_value<Rng, N, std::enable_if_t<(std::ranges::range<Rng> && N > 0),
        empty_t>> : md_range_value<std::ranges::range_value_t<Rng>, N - 1> {};



template<class Rng, size_t D>
using md_range_value_t = typename md_range_value<Rng, D>::type;
}


#include "type_traits/type_traits.inl"


namespace cth::type {}
