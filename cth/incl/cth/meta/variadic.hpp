#pragma once
#include <concepts>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "cth/meta/concepts.hpp"


#include "string.hpp"


namespace cth::mta {

template<size_t I, class... Ts>
struct at : std::tuple_element<I, std::tuple<Ts...>> {};

template<size_t I, class... Ts>
using at_t = at<I, Ts...>::type;
}


namespace cth::meta {

template<auto TCpt, class... Ts>
constexpr auto cpt_mask() {
    return std::array<bool, sizeof...(Ts)>{
        mta::satisfies<Ts, TCpt>...
    };
}


/**
 * @brief finds idx of the first type in `Ts...` satisfying `TCpt`
 * @tparam TCpt template concept to check
 * @tparam Ts types to find in
 * @return integer index
 */
template<auto TCpt, class... Ts>
consteval size_t cpt_find_idx() {
    auto constexpr mask = cpt_mask<TCpt, Ts...>();

    return std::distance(
        mask.begin(),
        std::ranges::find_if(mask, [](bool bit) { return bit; })
    );
}


/**
 * @brief Carries the first of `Ts` that satisfies `TCpt`
 *
 * @tparam TCpt template concept to check
 * @tparam Ts types to find in
 */
template<auto TCpt, class... Ts> requires(mta::any_satisfy<TCpt, Ts...>)
struct cpt_find : mta::at<cpt_find_idx<TCpt, Ts...>(), Ts...> {};

/**
 * @brief shorthand for @ref cpt_find
 */
template<auto TCpt, class... Ts>
using cpt_find_t = cpt_find<TCpt, Ts...>::type;

}


namespace cth::mta {

namespace dev {
    template<class T, class... Ts> requires(convertible_to_any<T, Ts...>)
    consteval auto convert_to_any_fn() {
        if constexpr(is_any_of<T, Ts...>)
            return std::type_identity<T>{};
        else
            return meta::cpt_find<
                CPT(convertible_from<T>),
                Ts...
            >{};
    }
}


template<class T, class... Ts> requires (convertible_to_any<T, Ts...>)
struct convert_to_any : decltype(dev::convert_to_any_fn<T, Ts...>()) {};


template<class T, class... Ts>
using convert_to_any_t = convert_to_any<T, Ts...>::type;

/**
 * @brief converts to the first type of `To... that is convertible from `From`, 
 *  does nothing if `From` is in `To...`
 * @tparam To possible conversions
 * @tparam From conversion type
 */
template<class From, class... To, class T> requires(convertible_to_any<T, To...>)
decltype(auto) to_convertible_from(T&& arg) {
    if constexpr(is_any_of<From, To...>)
        return static_cast<From>(arg);
    else
        return static_cast<convert_to_any_t<From, To...>>(arg);
}

/**
 * @brief shorthand for @ref to_convertible_from with U = T
 */
template<class... Ts, class T>
decltype(auto) to_convertible(T&& arg) {
    return mta::to_convertible_from<T, Ts...>(std::forward<T>(arg));
}

}


namespace cth::mta {



template<class T, class... Ts> requires(constructs_any_of<T, Ts...>)
consteval auto construct_any_from_fn() {
    if constexpr(convertible_to_any<T, Ts...>)
        return convert_to_any<T, Ts...>{};
    else
        return meta::cpt_find<
            CPT(std::constructible_from<T>),
            Ts...
        >{};

}


/**
 * Resolves to the first of `Ts...` that is constructible from `T`. 
 *  Tries to use @ref convert_to_any if possible
 * @tparam T constructor arg
 * @tparam Ts options for construction
 */
template<class T, class... Ts> requires (constructs_any_of<T, Ts...>)
struct construct_any_of : decltype(construct_any_from_fn<T, Ts...>()) {};

/**
 * @brief shorthand for @ref construct_any_of
 * @tparam T constructor arg
 * @tparam Ts options for construction
 */
template<class T, class... Ts>
using construct_any_from_t = construct_any_of<T, Ts...>::type;

/**
 * constructs the first type of `Ts...` that is constructible from `T`, 
 *  or calls @ref to_convertible_from<T, Ts...> if possible
 * @tparam Ts to any of Ts...
 * @tparam U from
 * @note prioritizes to_convertible<T, Ts...> if available
 */
template<class U, class... Ts, class T> requires(constructs_any_of<T, Ts...>)
auto to_constructible_from(T&& arg) {
    if constexpr(convertible_to_any<U, Ts...>)
        return cth::mta::to_convertible_from<U, Ts...>(std::forward<T>(arg));
    else
        return construct_any_from_t<U, Ts...>{arg};
}

/**
 * @brief shorthand for @ref to_constructible_from with `U = T`
 */
template<class... Ts, class T>
auto to_constructible(T&& arg) {
    return mta::to_constructible_from<T, Ts...>(std::forward<T>(arg));
}
}


namespace cth::mta {

namespace dev {

    template<class Tuple, class... Ts>
    struct unique_tuple_impl;

    template<class Tuple, class T, class... Ts> requires(is_any_of<T, Ts...>)
    struct unique_tuple_impl<Tuple, T, Ts...> : unique_tuple_impl<Tuple, Ts...> {};

    template<class... TupleTs, class T, class... Ts> requires(!is_any_of<T, Ts...>)
    struct unique_tuple_impl<std::tuple<TupleTs...>, T, Ts...> : unique_tuple_impl<std::tuple<TupleTs..., T>, Ts...> {};

    template<class Tuple>
    struct unique_tuple_impl<Tuple> {
        using types = Tuple;
    };
}



template<class... Ts>
struct unique_tuple : dev::unique_tuple_impl<std::tuple<>, Ts...> {};

template<class... Ts>
using unique_tuple_t = unique_tuple<Ts...>::types;

}


namespace cth::mta {
namespace dev {
    template<class Overload>
    struct resolution_node {
        Overload operator()(Overload);
    };

    template<class... Ts>
    struct overload_set : resolution_node<Ts>... {
        using resolution_node<Ts>::operator()...;
    };
}


/**
 * Resolves the correct function overload type for Overload from Overloads
 * @tparam T resolve target
 * @tparam Overloads options to resolve from
 */
template<class T, class... Overloads>
struct resolve_overload_from : std::invoke_result<dev::overload_set<Overloads...>, T> {};


/**
 * shorthand for @ref resolve_overload_from
 * @tparam T resolve target
 * @tparam Overloads options to resolve from
 */
template<class T, class... Overloads>
using resolve_overload_from_t = resolve_overload_from<T, Overloads...>::type;


/**
 * checks if overload resolves to any of Overloads via function overload resolution
 * @tparam T resolve target
 * @tparam Ts options to resolve from
 */
template<class T, class... Ts>
concept resolves_to_overloads = std::same_as<unique_tuple_t<Ts...>, std::tuple<Ts...>> && requires {
    typename resolve_overload_from<T, Ts...>::type;
};
}
