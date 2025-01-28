// ReSharper disable CppClangTidyCertDcl58Cpp
#pragma once
#include "types/variadic.hpp"

#include <format>
#include <ranges>


/**
 * @brief creates a format overload for the type
 * @param type type
 * @param to_string to_string(type) function
 * @note should only be used in global namespace context
 */
#define CTH_FORMAT_TYPE(type, to_string) \
    template<>\
    struct std::formatter<type> : std::formatter<int> { \
        constexpr auto parse(std::format_parse_context& ctx) { return std::formatter<int>::parse(ctx); } \
        template<typename FormatContext> \
        constexpr auto format(type const& obj, FormatContext& ctx) const { return std::format_to(ctx.out(), #type "{{{0}}}", to_string(obj)); } \
    }

/**
 * @brief creates a format overload for the concept
 * @param cpt concept
 * @param to_string to_string(type) function
 * @note should only be used in global namespace context
 */
#define CTH_FORMAT_CPT(cpt, to_string) \
    template<cpt T> requires(!cth::type::convertible_to_any<cth::type::pure_t<T>, std::string, std::string_view>) \
    struct std::formatter<T> : std::formatter<int> { \
        constexpr auto parse(std::format_parse_context& ctx) { return std::formatter<int>::parse(ctx); } \
        template<typename FormatContext> \
        auto format(T const& obj, FormatContext& ctx) const { return std::format_to(ctx.out(), #cpt "{{{0}}}", to_string(obj)); } \
    }

