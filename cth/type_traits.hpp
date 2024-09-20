#pragma once
#include <limits>
#include <string>
#include <type_traits>

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

//helpers

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


template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<class Rng, class T>
concept range_over = std::ranges::range<Rng> and std::same_as<std::ranges::range_value_t<Rng>, T>;

template<class T>
concept character = std::same_as<pure_t<T>, char> or std::same_as<pure_t<T>, wchar_t>;

template<class T>
concept n_character = std::same_as<pure_t<T>, char>;

template<class T>
concept w_character = std::same_as<pure_t<T>, wchar_t>;

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

//is_any_of

namespace cth::type {

template<class T, class... Ts>
concept is_any_of = any_satisfy<cpt(std::same_as<T>), Ts...>;

/**
 * \brief ::type is equal to first of Ts... that's equal to T or Fallback if none are
 * \tparam Fb (Fallback) if none of Ts... are equal to T
 */
template<class Fb, class T, class... Ts> struct fallback_any_of;

/**
 * @brief shortcut to @ref fallback_any_of::type
 */
template<class Fb, class T, class... Ts>
using fallback_any_of_t = typename fallback_any_of<Fb, T, Ts...>::type;

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



/**
 * \brief converts to the first type of Ts... that is the same as T
 * \tparam Ts to any of Ts...
 * \tparam T from
 */
template<typename T, typename... Ts> requires (is_any_of<T, Ts...>)
auto to_same_of(T&& arg);

/**
 * \brief converts to the first type of Ts... that is the same as T
 * \tparam Ts to any of Ts...
 * \tparam T from
 */
template<typename... Ts, typename T>
auto to_same(T&& arg);
}

//convert to any

namespace cth::type {
/**
 * @brief true if any of Ts... are convertible to T
 */
template<typename T, typename... Ts>
concept any_convertible_to = any_satisfy<cpt(std::convertible_to<T>), Ts...>;

/**
 * @brief true if T is convertible to any of Ts...
 */
template<typename T, typename... Ts>
concept convertible_to_any = (std::convertible_to<T, Ts> || ...);

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
 * \brief converts to the first type of Ts... that is convertible from T
 * \tparam Ts to any of Ts...
 * \tparam U from
 * \note prioritizes to_same_of<T, Ts...> if available
 */
template<typename U, typename... Ts, typename T> requires(convertible_to_any<T, Ts...>)
auto to_convertible_from(T&& arg);

/**
 * \brief converts to the first type of Ts... that is convertible from T
 * \tparam Ts to any of Ts...
 * \tparam T from
 * \note prioritizes to_same_of<T, Ts...> if available
 */
template<typename... Ts, typename T>
auto to_convertible(T&& arg);
}

namespace cth::type {

/**
 * @brief shortcut to @ref is_any_constructible_from_v
 */
template<typename T, typename... Ts>
concept any_constructible_from = any_satisfy<cpt(std::constructible_from<T>), Ts...>;



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

/**
 * @brief shortcut to @ref construct_any_from_t::type
 */
template<typename T, typename... Ts>
using construct_any_from_t = typename construct_any_from<T, Ts...>::type;



/**
 * \brief constructs the first type of Ts... that is constructible from T
 * \tparam Ts to any of Ts...
 * \tparam U from
 * \note prioritizes to_convertible<T, Ts...> if available
 */
template<typename U, typename... Ts, typename T> requires(any_constructible_from<T, Ts...>)
auto to_constructible_from(T&& arg);
/**
 * \brief constructs the first type of Ts... that is constructible from T
 * \tparam Ts to any of Ts...
 * \tparam T from
 * \note prioritizes to_convertible<T, Ts...> if available
 */
template<typename... Ts, typename T>
auto to_constructible(T&& arg);
}


//md_range

namespace cth::type {
namespace dev {
    constexpr size_t MAX_D = std::numeric_limits<size_t>::max();

    template<class T, size_t Max = 0> requires(!std::ranges::range<T>)
    consteval size_t dimensions(size_t n) { return n; }
    template<std::ranges::range Rng, size_t Max = MAX_D>
    consteval size_t dimensions(size_t n);
}

/**
 * @brief counts the dimensions of @ref Rng
 * @tparam Rng range
 * @tparam Max optional max search depth
 */
template<class Rng, size_t Max = dev::MAX_D>
consteval size_t dimensions();

/**
 * @brief checks if @ref Rng is of at least @ref D dimensions
 * @tparam Rng range
 * @tparam D dimensions
 */
template<class Rng, size_t D>
concept md_range = dimensions<Rng, D>() == D;

/**
 * @brief type trait to get the first non range type or the @ref D -th dimension range value type of @ref Rng
 * @tparam Rng range
 * @tparam D dimension (optional)
 */
template<class Rng, size_t D = dev::MAX_D, class = empty_t>
struct md_range_value;


/**
 * @brief shortcut to @ref md_range_value::type
 */
template<class Rng, size_t D = dev::MAX_D>
using md_range_value_t = typename md_range_value<Rng, D>::type;

/**
 * @brief shortcut to @ref md_range_value_t<Rng, 2>
 */
template<class Rng>
using range2d_value_t = md_range_value_t<Rng, 2>;


template<class Rng, class T, size_t D = dev::MAX_D>
concept md_range_over = std::same_as<T, md_range_value_t<Rng, D>>;

template<class Rng, auto TCpt, size_t D = dev::MAX_D>
concept md_range_over_cpt = satisfies<md_range_value_t<Rng, D>, TCpt>;


template<class Rng, auto TCpt>
concept range_over_cpt = md_range_over_cpt<Rng, TCpt, 1>;

template<class Rng, class T>
concept range2d_over = md_range_over<Rng, T, 2>;
template<class Rng, auto TCpt>
concept range2d_over_cpt = md_range_over_cpt<Rng, TCpt, 2>;

}


#include "type_traits/type_traits.inl"


namespace cth::type {}
