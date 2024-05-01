// ReSharper disable CppInconsistentNaming
#pragma once

#include <type_traits>

//it doesn't see that the macro needs std::string_view
// ReSharper disable once CppUnusedIncludeDirective
#include <string>


/**
 * \brief ... is the variable for the expression
 */
#define CTH_TYPE_GROUP(name, ...) \
template<typename T> static constexpr bool is_##name##_v = __VA_ARGS__; \
template<typename T> struct is_##name : std::bool_constant<is_##name##_v<T>> {};


namespace cth::type {
namespace dev {
    template<class T, T V>
    struct constant {
        static constexpr T value = V;

        using value_type = T;
        using type = constant;

        [[nodiscard]] constexpr operator value_type() const noexcept { return value; }

        [[nodiscard]] constexpr value_type operator()() const noexcept { return value; }
    };
}

template<bool V>
using bool_constant = dev::constant<bool, V>;

template<typename T, typename... Ts> struct is_any_of : bool_constant<std::disjunction_v<std::is_same<T, Ts>...>> {};
template<typename T, typename... Ts> constexpr bool is_any_of_v = is_any_of<T, Ts...>::value;
template<typename T, typename... Ts> requires (is_any_of_v<T, Ts...>)
struct any_of {
    using type = T;
};
template<typename T, typename... Ts> using any_of_t = typename any_of<T, Ts...>::type;



template<typename T, typename... Ts> struct is_convertible_to_any_of : bool_constant<(... || std::convertible_to<T, Ts>)> {};
template<typename T, typename... Ts> constexpr bool is_convertible_to_any_of_v = is_convertible_to_any_of<T, Ts...>::value;

//template<typename T, typename First, typename... Rest>
//struct is_convertible_to_any_of_helper {
//    using type = std::conditional_t<std::convertible_to<T, First>, First, typename is_convertible_to_any_of_helper<T, Rest...>::type>;
//};
//
//template<typename T, typename First>
//struct is_convertible_to_any_of_helper<T, First> {
//    //static_assert(std::convertible_to<T, First>, "No type in the list is convertible to T");
//    using type = First;
//};
//
//template<typename T, typename... Ts> requires (is_convertible_to_any_of_v<T, Ts...>)
//struct convertible_to_any_of {
//    using type = typename is_convertible_to_any_of_helper<T, Ts...>::type;
//};
//template<typename T, typename... Ts> using convertible_to_any_of_t = typename is_convertible_to_any_of_helper<T, Ts...>::type;
template<typename T, typename... Ts>
struct convertible_to_any_of;

template<typename T, typename First, typename... Rest>
struct convertible_to_any_of<T, First, Rest...> {
    using type = std::conditional_t<std::is_convertible_v<T, First>, First, typename convertible_to_any_of<T, Rest...>::type>;
};

template<typename T>
struct convertible_to_any_of<T> {
    using type = void;
    static_assert(std::is_same_v<T, void>, "None of the list types are convertible to T");
};

template<typename T, typename... Ts>
using convertible_to_any_of_t = typename convertible_to_any_of<T, Ts...>::type;

template<typename T, typename Ts> requires (is_any_of_v<T, Ts>)
auto to_same_of(T&& arg) { return std::forward<T>(arg); }

template<typename T, typename... Ts>
auto to_convertible(T&& arg) {
    if constexpr(is_any_of_v<T, Ts...>) return cth::type::to_same_of<T, Ts...>(std::forward<T>(arg));
    else return static_cast<convertible_to_any_of_t<T, Ts...>>(std::move(arg));
}

int x = to_convertible<int, float, double>(10);

//char
CTH_TYPE_GROUP(wchar, std::_Is_any_of_v<std::decay_t<T>, wchar_t>)
CTH_TYPE_GROUP(nchar, std::_Is_any_of_v<std::decay_t<T>, char>)
CTH_TYPE_GROUP(char, is_nchar_v<T> || is_wchar_v<T>)

////string
//CTH_TYPE_GROUP(nstring, std::is_convertible_v<T, string> || std::is_constructible_v<T, string>)
//CTH_TYPE_GROUP(wstring, std::is_convertible_v<T, string> || std::is_constructible_v<T, string>)
//CTH_TYPE_GROUP(string, is_nstring_v<T> || is_wstring_v<T>)



//string
CTH_TYPE_GROUP(string_view_convertable, std::is_convertible_v<T, std::wstring_view> || std::is_convertible_v<T, std::string_view>)



}
