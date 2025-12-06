#pragma once
#include <concepts>
#include <tuple>
#include <type_traits>

namespace cth::type {
template<size_t I, class... Ts>
using at_t = decltype(std::get<I, std::tuple<Ts...>>(std::declval<std::tuple<Ts...>>()));
}

//is_any_of

namespace cth::type {

template<class T, class... Ts>
concept any_of = (std::same_as<T, Ts> || ...);

/**
 * \brief ::type is equal to first of Ts... that's equal to T or Fallback if none are
 * \tparam Fb (Fallback) if none of Ts... are equal to T
 */
template<class Fb, class T, class... Ts> struct fallback_any_of_trait;

/**
 * @brief shortcut to @ref fallback_any_of::type
 */
template<class Fb, class T, class... Ts>
using fallback_any_of_t = typename fallback_any_of_trait<Fb, T, Ts...>::type;

/**
 * \brief ::type is equal to first of Ts... that's equal to T
 */
template<class T, class... Ts>
struct any_of_trait;


/**
 * @brief shortcut to @ref any_of::type
 */
template<class T, class... Ts>
using any_of_t = typename any_of_trait<T, Ts...>::type;



/**
 * \brief converts to the first type of Ts... that is the same as T
 * \tparam Ts to any of Ts...
 * \tparam T from
 */
template<typename T, typename... Ts> requires (any_of<T, Ts...>)
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
concept any_convertible_to = (std::convertible_to<Ts, T> || ...);

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
struct fallback_convert_to_any_trait;

/**
 * @brief shortcut to @ref fallback_convert_to_any_t::type
 */
template<class Fb, class T, class... Ts>
using fallback_convert_to_any_t = typename fallback_convert_to_any_trait<Fb, T, Ts...>::type;

/**
 * \brief ::type is equal to first of Ts... that's convertible from T
 * \tparam T from
 * \tparam Ts to any of Ts...
 * \note prioritizes any_of_t<T, Ts...> if available
 */
template<typename T, typename... Ts>
struct convert_to_any_trait;


/**
 *@brief shortcut to @ref convert_to_any_t::type
 */
template<typename T, typename... Ts>
using convert_to_any_t = typename convert_to_any_trait<T, Ts...>::type;

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
concept any_constructible_from = (std::constructible_from<Ts, T> || ...);



/**
 * \brief ::type is equal to first of Ts... that's constructible from T or Fallback if none are 
 * \tparam Fallback if none of Ts... are constructible from T
 * \tparam T from
 * \tparam Ts to any of Ts...
 * \note prioritizes convert_any_from_t<T, Ts...> if available
 */
template<typename Fallback, typename T, typename... Ts>
struct fallback_construct_any_from_trait;

/**
 * \brief ::type is equal to first type of Ts... that is constructible from T
 * \tparam T from
 * \tparam Ts to any of Ts...
 * \note prioritizes convert_any_from_t<T, Ts...> if available
 */
template<typename T, typename... Ts>
struct construct_any_from_trait;

/**
 * @brief shortcut to @ref construct_any_from_t::type
 */
template<typename T, typename... Ts>
using construct_any_from_t = typename construct_any_from_trait<T, Ts...>::type;

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

#include "inl/typ_variadic.inl"
