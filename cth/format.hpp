// ReSharper disable CppClangTidyCertDcl58Cpp
#pragma once
#include "string.hpp"

#include <format>
#include <ranges>

template<std::ranges::range T> requires(!cth::type::convertible_to_any<T, std::string>)
struct std::formatter<T> : std::formatter<int> {
    constexpr auto parse(format_parse_context& ctx) { return std::formatter<int>::parse(ctx); }

    template<typename FormatContext>
    auto format(T const& obj, FormatContext& ctx) const { return format_to(ctx.out(), "{}", cth::str::to_string(obj)); }
};
