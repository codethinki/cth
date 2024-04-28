#pragma once

#include <type_traits>

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
    struct Constant {
        static constexpr T value = V;

        typedef T value_type;
        typedef Constant type;

        [[nodiscard]] constexpr operator value_type() const noexcept { return value; }

        [[nodiscard]] constexpr value_type operator()() const noexcept { return value; }
    };
}


using namespace std;


template<bool V>
using bool_constant = dev::Constant<bool, V>;
//char
CTH_TYPE_GROUP(wchar, _Is_any_of_v<decay_t<T>, wchar_t>)
CTH_TYPE_GROUP(nchar, _Is_any_of_v<decay_t<T>, char>)
CTH_TYPE_GROUP(char, is_nchar_v<T> || is_wchar_v<T>)

////string
//CTH_TYPE_GROUP(nstring, std::is_convertible_v<T, string> || std::is_constructible_v<T, string>)
//CTH_TYPE_GROUP(wstring, std::is_convertible_v<T, string> || std::is_constructible_v<T, string>)
//CTH_TYPE_GROUP(string, is_nstring_v<T> || is_wstring_v<T>)




//string
CTH_TYPE_GROUP(string_view_convertable, std::is_convertible_v<T, wstring_view> || std::is_convertible_v<T, string_view>)



}
