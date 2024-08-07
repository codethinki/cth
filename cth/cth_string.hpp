#pragma once
//you have to define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING in the preprocessor or source io for this to work
#include "cth_concept.hpp"

#include <codecvt>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <vector>



namespace cth::str::conv {

namespace dev {
    inline static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter{};
}

[[nodiscard]] inline std::string toN(std::wstring_view const str) { return dev::converter.to_bytes(str.data()); }
[[nodiscard]] inline std::wstring toW(std::string_view const str) { return dev::converter.from_bytes(str.data()); }

[[nodiscard]] inline std::wstring wide(std::string_view const nstring) { return dev::converter.from_bytes(nstring.data()); }
[[nodiscard]] inline std::string narrow(std::wstring_view const wstring) { return dev::converter.to_bytes(wstring.data()); }
} // namespace cth::str::conv


namespace cth::str {
template<std::ranges::range Rng> requires(!std::ranges::range<std::ranges::range_value_t<Rng>>)
[[nodiscard]] std::string to_string(Rng const& range) {
    if(std::ranges::empty(range)) return "[ ]";

    std::string str = "[";
    for(auto&& element : range) str += std::format("{0}, ", element);
    str.pop_back();
    str.back() = ']';
    return str;
}

template<std::ranges::range Rng> requires(std::ranges::range<std::ranges::range_value_t<Rng>>)
[[nodiscard]] std::string to_string(Rng const& range_of_ranges) {
    std::string str = "[";
    for(auto const& range : range_of_ranges) str += std::format<std::string_view>("{0}, ", cth::str::to_string(range));
    str.pop_back();
    str.back() = ']';
    return str;
}

template<cth::type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(std::string_view const str) {
    T num = 0;
    size_t i = 0;
    for(; i < str.size(); i++) {
        char const c = str[i];
        if(c >= '0' && c <= '9') num = num * 10 + c - '0';
        else break;
    }
    if(i == str.size()) return num;

    if constexpr(std::is_floating_point_v<T>) {
        if(str[i++] != '.') goto failed;

        T d;
        d = 10;
        for(; i < str.size(); i++) {
            char const c = str[i];
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
[[nodiscard]] constexpr std::optional<T> to_num(std::wstring_view const str) {
    T num = 0;
    size_t i = 0;
    for(; i < str.size(); i++) {
        wchar_t const c = str[i];
        if(c >= L'0' && c <= L'9') num = num * 10 + c - L'0';
        else break;
    }
    if(i == str.size()) return num;

    if constexpr(!std::is_floating_point_v<T>) return std::nullopt;

    if(str[i++] != L'.') return std::nullopt;

    T d = 10;
    for(; i < str.size(); i++) {
        wchar_t const c = str[i];
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
[[nodiscard]] auto split(T const& str, U const& delimiter) {
    auto const view = type::to_constructible<std::string_view, std::wstring_view>(str);
    using char_t = typename decltype(view)::value_type;
    using ret_t = std::vector<std::basic_string<char_t>>;

    auto const d = type::to_constructible_from<std::decay_t<U>, char_t, std::basic_string_view<char_t>>(delimiter);

    return view | std::views::split(d)
        | std::views::filter([](auto const string_part) { return !string_part.empty(); })
        | std::ranges::to<ret_t>();
}



} // namespace cth::str



namespace cth::expr::str {

template<type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(std::string_view str);
template<type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(std::wstring_view str);

template<type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(std::string_view const str) { return cth::str::to_num<T>(str); }
template<type::arithmetic T>
[[nodiscard]] constexpr std::optional<T> to_num(std::wstring_view const str) { return cth::str::to_num<T>(str); }


} // namespace cth::expr::str
