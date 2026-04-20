// ReSharper disable CppClangTidyCertDcl58Cpp
#pragma once
#include "cth/meta/concepts.hpp"
#include <concepts>
#include <format>
#include <tuple>
#include <type_traits>
#include <utility>

/**
 * @brief creates a format overload for a type wrapping another, inheriting the parsing properties from the base type
 * @param type to make formattable
 * @param accessor function to access the aliased type
 * @attention must only be used in global namespace context
 */
#define CTH_FORMAT_ALIAS(type, base_type, accessor)                                 \
    template<>                                                                      \
    struct std::formatter<type> : std::formatter<base_type> {                       \
        template<class FormatContext>                                               \
        [[nodiscard]] cxpr auto format(type const& obj, FormatContext& ctx) const { \
            /* Use the base formatter logic, but pass the member */                 \
            return std::formatter<base_type>::format(accessor(obj), ctx);           \
        }                                                                           \
    }


/**
 * @brief creates a format overload for a class
 * @param type The class/struct type to format
 * @param fmt_string The format string (e.g., "ID: {}, Val: {}")
 * @param tie_func An expression involving 'obj' that returns a tuple/tie
 */
#define CTH_FORMAT_CLASS(type, fmt_string, tie_func)                                                \
template<>                                                                                          \
struct std::formatter<type> {                                                                       \
    [[nodiscard]] constexpr auto parse(std::format_parse_context& ctx) {                            \
        return ctx.begin();                                                                         \
    }                                                                                               \
                                                                                                    \
    template<class FormatContext>                                                                   \
    [[nodiscard]] constexpr auto format(type const& obj, FormatContext& ctx) const {                \
        using T = decltype(tie_func(std::declval<type>()));                                          \
                                                                                                    \
        if constexpr(!cth::mta::tuple_like<T> && std::formattable<T, char>)                         \
            return std::format_to(ctx.out(), fmt_string, tie_func(obj));                            \
        else                                                                                        \
            /* Use a templated lambda to unpack the tie into format_to */                           \
            return std::apply(                                                                      \
                [&ctx]<class... Args>(Args&&... args) {                                             \
                    return std::format_to(ctx.out(), fmt_string, std::forward<Args>(args)...);      \
                },                                                                                  \
                tie_func(obj)                                                                       \
            );                                                                                      \
    }                                                                                               \
}


/**
 * @brief creates a format overload for the concept
 * @param cpt concept
 * @param to_formattable to_formattable(type) function
 * @attention should only be used in global namespace context
 */
#define CTH_FORMAT_CPT(cpt, to_formattable)                                                                  \
    template<cpt T>                                                                                          \
    struct std::formatter<T> : std::formatter<int> {                                                         \
        [[nodiscard]] cxpr auto parse(std::format_parse_context& ctx) {                                      \
            return std::formatter<int>::parse(ctx);                                                          \
        }                                                                                                    \
        template<class FormatContext>                                                                        \
        [[nodiscard]] cxpr auto format(T const& obj, FormatContext& ctx) const {                             \
            auto value = to_formattable(obj);                                                                \
            static_assert(                                                                                   \
                !std::same_as<std::decay_t<decltype(value)>, void>,                                          \
                "parse function must not return void"                                                        \
            );                                                                                               \
            return std::format_to(ctx.out(), #cpt "{{{0}}}", std::move(value));                              \
        }                                                                                                    \
    }
