// ReSharper disable CppClangTidyCertDcl58Cpp
#pragma once
#include "cth/meta/concepts.hpp"
#include <concepts>
#include <format>
#include <tuple>
#include <type_traits>
#include <utility>

/**
 * creates a `std::formatter` of `type` that formats like `base_type`
 * @details parses like `std::formatter<base_type>`
 * @param type to make formattable
 * @param base_type aliased type, `type` wraps it
 * @param accessor function that gets the `base_type` value of a `type`
 * @attention use it in the global namespace only
 */
#define CTH_FORMAT_ALIAS(type, base_type, accessor)                                 \
    template<>                                                                      \
    struct std::formatter<type> : std::formatter<base_type> {                       \
        template<class FormatContext>                                               \
        [[nodiscard]] constexpr auto format(type const& obj, FormatContext& ctx) const { \
            /* Use the base formatter logic, but pass the member */                 \
            return std::formatter<base_type>::format(accessor(obj), ctx);           \
        }                                                                           \
    }


/**
 * creates a `std::formatter` of a class
 * @details accepts no format specifiers
 * @param type class or struct to format
 * @param fmt_string format string for the members, for example `"ID: {}, Val: {}"`
 * @param tie_func function that gets the members of a `type` as a tuple, called with a `type const&`
 * @attention use it in the global namespace only
 */
#define CTH_FORMAT_CLASS_CUSTOM(type, fmt_string, tie_func)                                         \
template<>                                                                                          \
struct std::formatter<type> {                                                                       \
    [[nodiscard]] constexpr auto parse(std::format_parse_context& ctx) {                            \
        return ctx.begin();                                                                         \
    }                                                                                               \
                                                                                                    \
    template<class FormatContext>                                                                   \
    [[nodiscard]] constexpr auto format(type const& obj, FormatContext& ctx) const {                \
        using T = decltype(tie_func(std::declval<type>()));                                         \
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
 * creates a `std::formatter` of a class
 * @details formats as `type{<fmt_string>}`
 * @param type class or struct to format
 * @param fmt_string format string for the members, for example `"ID: {}, Val: {}"`
 * @param tie_func function that gets the members of a `type` as a tuple, called with a `type const&`
 * @attention use it in the global namespace only
 */
#define CTH_FORMAT_CLASS(type, fmt_string, tie_func)\
    CTH_FORMAT_CLASS_CUSTOM(type, #type "{{" fmt_string "}}", tie_func)

/**
 * creates a `std::formatter` of all types that satisfy a concept
 * @details formats as `cpt{<value>}`, parses like `std::formatter<int>`
 * @param cpt concept
 * @param to_formattable function that gets a value of `cpt` as a formattable value
 * @pre `to_formattable()` does not return `void`
 * @attention use it in the global namespace only
 */
#define CTH_FORMAT_CPT(cpt, to_formattable)                                                                  \
    template<cpt T>                                                                                          \
    struct std::formatter<T> : std::formatter<int> {                                                         \
        [[nodiscard]] constexpr auto parse(std::format_parse_context& ctx) {                                 \
            return std::formatter<int>::parse(ctx);                                                          \
        }                                                                                                    \
        template<class FormatContext>                                                                        \
        [[nodiscard]] constexpr auto format(T const& obj, FormatContext& ctx) const {                        \
            auto value = to_formattable(obj);                                                                \
            static_assert(                                                                                   \
                !std::same_as<std::decay_t<decltype(value)>, void>,                                          \
                "parse function must not return void"                                                        \
            );                                                                                               \
            return std::format_to(ctx.out(), #cpt "{{{0}}}", std::move(value));                              \
        }                                                                                                    \
    }
