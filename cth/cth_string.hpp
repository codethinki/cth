#pragma once
//you have to define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING in the preprocessor or source file for this to work
#include "cth_concepts.hpp"

#include <codecvt>

#include <optional>
#include <ranges>
#include <string>
#include <vector>



namespace cth::str::conv {

namespace dev {
    inline static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter{};
}

[[nodiscard]] inline std::string toN(const std::wstring_view str) { return dev::converter.to_bytes(str.data()); }
[[nodiscard]] inline std::wstring toW(const std::string_view str) { return dev::converter.from_bytes(str.data()); }

[[nodiscard]] inline std::wstring wide(const std::string_view nstring) { return dev::converter.from_bytes(nstring.data()); }
[[nodiscard]] inline std::string narrow(const std::wstring_view wstring) { return dev::converter.to_bytes(wstring.data()); }
} // namespace cth::str::conv


namespace cth::str {


template<cth::type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(const std::string_view str) {
    T num = 0;
    size_t i = 0;
    for(; i < str.size(); i++) {
        const char c = str[i];
        if(c >= '0' && c <= '9') num = num * 10 + c - '0';
        else break;
    }
    if(i == str.size()) return num;

    if constexpr(std::is_floating_point_v<T>) {
        if(str[i++] != '.') goto failed;

        T d;
        d = 10;
        for(; i < str.size(); i++) {
            const char c = str[i];
            if(c >= '0' && c <= '9') num += (c - '0') / d;
            else goto failed;
            d *= 10;
        }
        return num;
    failed:;
    }


    return std::nullopt;
}
template<type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(const std::wstring_view str) {
    T num = 0;
    size_t i = 0;
    for(; i < str.size(); i++) {
        const wchar_t c = str[i];
        if(c >= L'0' && c <= L'9') num = num * 10 + c - L'0';
        else break;
    }
    if(i == str.size()) return num;

    if constexpr(!std::is_floating_point_v<T>) return std::nullopt;

    if(str[i++] != L'.') return std::nullopt;

    T d = 10;
    for(; i < str.size(); i++) {
        const wchar_t c = str[i];
        if(c >= L'0' && c <= L'9') num += (c - L'0') / d;
        else return std::nullopt;
        d *= 10;
    }
    return num;
}

//namespace dev {
/**
 * \brief splits a string into a vector of strings
 * \tparam U the delimiter type
 */
template<type::string_view_convertable T, type::literal U>
[[nodiscard]] auto split(const T& str, const U& delimiter) {
    const auto view = type::to_constructible<std::string_view, std::wstring_view>(str);
    using char_t = typename decltype(view)::value_type;
    using ret_t = std::vector<std::basic_string<char_t>>;

    const auto d = type::to_constructible_from<std::decay_t<U>, char_t, std::basic_string_view<char_t>>(delimiter);

    return view | std::views::split(d)
        | std::views::filter([](const auto string_part) { return !string_part.empty(); })
        | std::ranges::to<ret_t>();
}



} // namespace cth::str



namespace cth::expr::str {

template<type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(std::string_view str);
template<type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(std::wstring_view str);

template<type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(const std::string_view str) { return cth::str::to_num<T>(str); }
template<type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(const std::wstring_view str) { return cth::str::to_num<T>(str); }


} // namespace cth::expr::str
