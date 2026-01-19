module;
#include "cthm/macro/format.hpp"
#include "cthm/macro/utility.hpp"

#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

export module cth.string;
import cth.meta.concepts;
import cth.meta.ranges;
import cth.meta.traits;

import cth.meta.variadic;


export namespace cth::str {
template<class T>
concept char_formattable = cth::mta::any_constructible_from<T, std::string_view, std::string> ;
}

export namespace cth::str {
[[nodiscard]] cxpr std::vector<char const*> to_c_str_vector(std::span<std::string const> const& str_vec) {
    std::vector<char const*> charVec(str_vec.size());
    std::ranges::transform(str_vec, charVec.begin(), [](std::string const& str) { return str.c_str(); });

    return charVec;
}


template<cth::mta::arithmetic T>
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



/**
 * @brief formats ranges to string
 * @tparam Rng must satisfy rng::static_dim_rng<Rng>
 */
template<cth::rng::viewable_rng Rng> requires (cth::rng::static_dim_rng<cth::mta::rcvr_t<Rng>>)
[[nodiscard]] cxpr std::string to_string(Rng&& range) {
    static cxpr bool MD_RANGE = cth::mta::md_range<Rng, 2>;

    declauto rng = cth::rng::to_viewable(range);
    using rng_t = decltype(rng);

    if(std::ranges::empty(rng)) return {};


    std::string string = "[";
    for(auto&& element : std::forward<rng_t>(rng)) {
        using E = decltype(element);
        if constexpr(MD_RANGE)
            string.insert_range(string.end(), std::format("{}, ", cth::str::to_string(std::forward<E>(element))));
        else {
            static_assert(std::formattable<E, char>, "range value type is not formattable");
            string.insert_range(string.end(), std::format("{}, ", std::forward<E>(element)));
        }
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
    cth::mta::convertible_to_any<std::string_view, std::wstring_view> T,
    cth::mta::convertible_to_any<std::string_view, std::wstring_view, char, wchar_t> U
>
[[nodiscard]] cxpr auto split(T const& str, U const& delimiter) {
    auto const view = cth::mta::to_constructible<std::string_view, std::wstring_view>(str);
    using char_t = decltype(view)::value_type;
    using ret_t = std::vector<std::basic_string<char_t>>;

    auto const d = cth::mta::to_constructible_from<std::decay_t<U>, char_t, std::basic_string_view<char_t>>(delimiter);

    return view | std::views::split(d)
        | std::views::filter([](auto string_part) { return !string_part.empty(); })
        | std::ranges::to<ret_t>();
}

} // namespace cth::str


export namespace cth::str {
template<class T>
concept printable_rng =
    cth::rng::viewable_rng<T>
    && cth::rng::static_dim_rng<cth::mta::rcvr_t<T>>
    && !cth::mta::any_constructible_from<T, std::string_view, std::string>;
}


export CTH_FORMAT_CPT(cth::str::printable_rng, cth::str::to_string);



export namespace cth::expr::str {

template<cth::mta::arithmetic T>
[[nodiscard]] cxpr std::optional<T> to_num(std::string_view str, int base) { return cth::str::to_num<T>(str, base); }


} // namespace cth::expr::str
