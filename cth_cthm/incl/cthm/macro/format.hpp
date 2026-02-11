// ReSharper disable CppClangTidyCertDcl58Cpp
#pragma once

#include <format>


/**
 * @brief creates a format overload for the type
 * @param type type
 * @param to_formattable to_formattable(type) function
 * @attention should only be used in global namespace context
 */
#define CTH_FORMAT_TYPE(type, to_formattable)                                                                \
    template<>                                                                                               \
    struct std::formatter<type> : std::formatter<int> {                                                      \
        [[nodiscard]] cxpr auto parse(std::format_parse_context& ctx) {                                      \
            return std::formatter<int>::parse(ctx);                                                          \
        }                                                                                                    \
        template<class FormatContext>                                                                        \
        [[nodiscard]] auto format(type const& obj, FormatContext& ctx) const {                               \
            return std::format_to(ctx.out(), #type "{{{0}}}", to_formattable(obj));                          \
        }                                                                                                    \
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
