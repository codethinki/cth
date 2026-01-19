module;
#include <concepts>
#include <tuple>
#include <type_traits>

export module cth.meta.variadic:decl;


export namespace cth::mta {
template<size_t I, class... Ts>
using at_t = decltype(std::get<I, std::tuple<Ts...>>(std::declval<std::tuple<Ts...>>()));
}


//is_any_of

export namespace cth::mta {

template<class T, class... Ts>
concept any_of = (std::same_as<T, Ts> || ...);


template<class Opt, class T, class... Ts>
struct opt_any_of_trait {
    using type = std::conditional_t<any_of<T, Ts...>, T, Opt>;
};

template<class Opt, class T, class... Ts>
using opt_any_of_t = opt_any_of_trait<Opt, T, Ts...>::type;

template<class T, class... Ts>
struct any_of_trait {
    using type = opt_any_of_trait<void, T, Ts...>;
    static_assert(!any_of<T, Ts...>, "none of the types match");
};

/**
 * @brief shortcut to @ref any_of::type
 */
template<class T, class... Ts>
using any_of_t = any_of_trait<T, Ts...>::type;



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

export namespace cth::mta {
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
using fallback_convert_to_any_t = fallback_convert_to_any_trait<Fb, T, Ts...>::type;

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
using convert_to_any_t = convert_to_any_trait<T, Ts...>::type;

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


export namespace cth::mta {

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
using construct_any_from_t = construct_any_from_trait<T, Ts...>::type;

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

