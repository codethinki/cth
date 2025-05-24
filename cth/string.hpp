#pragma once
#include "macro.hpp"

#include "string/format.hpp"
#include "types/ranges.hpp"
#include "types/variadic.hpp"

#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <vector>


namespace cth::str {
template<class T>
concept char_formattable = cth::type::any_constructible_from<T, std::string_view, std::string> ;
}

namespace cth::str {
[[nodiscard]] cxpr std::vector<char const*> to_c_str_vector(std::span<std::string const> const& str_vec) {
    std::vector<char const*> charVec(str_vec.size());
    std::ranges::transform(str_vec, charVec.begin(), [](std::string const& str) { return str.c_str(); });

    return charVec;
}


template<cth::type::arithmetic T>
[[nodiscard]] cxpr std::optional<T> to_num(std::string_view str) {
    T num = 0;
    size_t i = 0;
    for(; i < str.size(); i++) {
        char const c = str[i];
        if(c >= '0' && c <= '9') num = num * 10 + c - '0';
        else break;
    }
    if(i == str.size()) return num;

    if cxpr(std::is_floating_point_v<T>) {
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



template<std::ranges::range Rng> requires std::formattable<type::md_range_value_t<std::remove_cvref_t<Rng>>, char>
[[nodiscard]] cxpr std::string to_string(Rng&& rng) {
    if(std::ranges::empty(rng)) return {};

    static cxpr bool MD_RANGE = type::md_range<Rng, 2>;

    std::string string = "[";

    for(auto&& element : std::forward<Rng>(rng)) {
        using E = decltype(element);
        if constexpr(MD_RANGE)
            string.insert_range(string.end(), std::format("{}, ", cth::str::to_string(std::forward<E>(element))));
        else
            string.insert_range(string.end(), std::format("{}, ", std::forward<E>(element)));
    }

    string.pop_back();
    string.back() = ']';
    return string;
}


/**
 * \brief splits a string into a vector of strings
 * \tparam U the delimiter type
 */
template<
    type::convertible_to_any<std::string_view, std::wstring_view> T,
    type::convertible_to_any<std::string_view, std::wstring_view, char, wchar_t> U
>
[[nodiscard]] cxpr auto split(T const& str, U const& delimiter) {
    auto const view = type::to_constructible<std::string_view, std::wstring_view>(str);
    using char_t = typename decltype(view)::value_type;
    using ret_t = std::vector<std::basic_string<char_t>>;

    auto const d = type::to_constructible_from<std::decay_t<U>, char_t, std::basic_string_view<char_t>>(delimiter);

    return view | std::views::split(d)
        | std::views::filter([](auto string_part) { return !string_part.empty(); })
        | std::ranges::to<ret_t>();
}



} // namespace cth::str


namespace cth::str {
template<class T>
concept non_string_rng = std::ranges::range<T> && !cth::type::any_constructible_from<T, std::string_view, std::string>;
}


CTH_FORMAT_CPT(cth::str::non_string_rng, cth::str::to_string);



namespace cth::expr::str {

template<type::arithmetic T>
[[nodiscard]] cxpr std::optional<T> to_num(std::string_view str, int base) { return cth::str::to_num<T>(str, base); }


} // namespace cth::expr::str
