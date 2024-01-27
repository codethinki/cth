#pragma once

#include <string>
#include <type_traits>
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
CTH_TYPE_GROUP(wchar, _Is_any_of_v<remove_cv_t<T>, wchar_t>)
CTH_TYPE_GROUP(nchar, _Is_any_of_v<remove_cv_t<T>, char>)
CTH_TYPE_GROUP(char, is_nchar_v<T> || is_wchar_v<T>)

//char_p
CTH_TYPE_GROUP(nchar_p, _Is_any_of_v<decay_t<remove_cv_t<T>>, const char*, char*>)
CTH_TYPE_GROUP(wchar_p, _Is_any_of_v<decay_t<remove_cv_t<T>>, const wchar_t*, wchar_t*>)
CTH_TYPE_GROUP(char_p, is_wchar_p_v<T> || is_nchar_p_v<T>)



//string
CTH_TYPE_GROUP(nstring, _Is_any_of_v<remove_cv_t<T>, string>)
CTH_TYPE_GROUP(wstring, _Is_any_of_v<remove_cv_t<T>, wstring>)
CTH_TYPE_GROUP(string, _Is_any_of_v<remove_cv_t<T>, string, wstring>)

//textual
CTH_TYPE_GROUP(ntextual, is_nstring_v<T> || is_nchar_p_v<T>)
CTH_TYPE_GROUP(wtextual, is_wstring_v<T> || is_wchar_p_v<T>)
CTH_TYPE_GROUP(textual, is_string_v<T> || is_char_p_v<T>)

//literal
CTH_TYPE_GROUP(wliteral, is_wstring_v<T> || is_wchar_p_v<T> || is_wchar_v<T>)
CTH_TYPE_GROUP(nliteral, is_nstring_v<T> || is_nchar_p_v<T> || is_nchar_v<T>)
CTH_TYPE_GROUP(literal, is_nliteral_v<T> || is_wliteral_v<T>)



//string_view (not in literal or textual because it's not the actual string, it doesn't own the memory)
CTH_TYPE_GROUP(string_view, _Is_any_of_v<remove_cv_t<T>, string_view, wstring_view>)




//stream
CTH_TYPE_GROUP(ostream, _Is_any_of_v<remove_cv_t<T>, ostream, wostream>);


}
