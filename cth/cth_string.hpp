#pragma once
//you have to define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING in the preprocessor or source file for this to work
#include "cth_concepts.hpp"

#include <codecvt>

#include <optional>
#include <ranges>
#include <string>
#include <vector>



namespace cth::str::conv {
using namespace std;

namespace dev {
    inline static wstring_convert<codecvt_utf8_utf16<wchar_t>> converter{};
}

[[nodiscard]] inline string toN(const wstring_view str) { return dev::converter.to_bytes(str.data()); }
[[nodiscard]] inline wstring toW(const string_view str) { return dev::converter.from_bytes(str.data()); }

[[nodiscard]] inline wstring wide(const string_view nstring) { return dev::converter.from_bytes(nstring.data()); }
[[nodiscard]] inline string narrow(const wstring_view wstring) { return dev::converter.to_bytes(wstring.data()); }
} // namespace cth::str::conv


namespace cth::str {
using namespace std;

template<type::arithmetic_t T>
[[nodiscard]] constexpr optional<T> to_num(const string_view str) {
    T num = 0;
    size_t i = 0;
    for(; i < str.size(); i++) {
        const char c = str[i];
        if(c >= '0' && c <= '9') num = num * 10 + c - '0';
        else break;
    }
    if(i == str.size()) return num;

    if constexpr(type::is_floating_point_v<T>) {
        if(str[i++] != '.') goto failed;

        T d = 10;
        for(; i < str.size(); i++) {
            const char c = str[i];
            if(c >= '0' && c <= '9') num += (c - '0') / d;
            else goto failed;
            d *= 10;
        }
        return num;
    }


failed:;
    return std::nullopt;
}
template<type::arithmetic_t T>
[[nodiscard]] constexpr std::optional<T> to_num(const std::wstring_view str) {
    T num = 0;
    size_t i = 0;
    for(; i < str.size(); i++) {
        const wchar_t c = str[i];
        if(c >= L'0' && c <= L'9') num = num * 10 + c - L'0';
        else break;
    }
    if(i == str.size()) return num;

    if constexpr(!type::is_floating_point_v<T>) return std::nullopt;

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

namespace dev {
    /**
     * \brief splits a string into a vector of strings
     * \tparam V the delimiter type
     */
    template<type::string_t T, type::string_view_t U, typename V>
    [[nodiscard]] vector<T> split(const U str, const V delimiter) {
        vector<T> result = str
            | views::split(delimiter)
            | views::filter([](const auto string_part) { return !string_part.empty(); })
            | ranges::to<vector<T>>();

        return result;
    }
} // namespace dev

inline vector<string> split(const string_view str, const string_view delimiter) { return dev::split<string>(str, delimiter); }
inline vector<string> split(const string_view str, const char delimiter) { return dev::split<string>(str, delimiter); }
inline vector<wstring> split(const wstring_view str, const wstring_view delimiter) { return dev::split<wstring>(str, delimiter); }
inline vector<wstring> split(const wstring_view str, const wchar_t delimiter) { return dev::split<wstring>(str, delimiter); }


} // namespace cth::str



namespace cth::expr::str {
using namespace std;
template<type::arithmetic_t T>
[[nodiscard]] constexpr optional<T> to_num(string_view str);
template<type::arithmetic_t T>
[[nodiscard]] constexpr std::optional<T> to_num(std::wstring_view str);

template<type::arithmetic_t T>
[[nodiscard]] constexpr optional<T> to_num(const string_view str) { return cth::str::to_num<T>(str); }
template<type::arithmetic_t T>
[[nodiscard]] constexpr std::optional<T> to_num(const std::wstring_view str) { return cth::str::to_num<T>(str); }


} // namespace cth::expr::str
