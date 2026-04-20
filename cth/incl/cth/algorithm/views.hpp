#pragma once
#include "cth/algorithm/alg_utility.hpp"

#include "cth/macro.hpp"

#include <assert.h>
#include <format>
#include <ranges>


namespace cth::views {
struct to_ptr_range_fn : cth::ranges::piped_fn {
    template<std::ranges::viewable_range Rng> requires std::is_lvalue_reference_v<std::ranges::range_reference_t<Rng>>
    cxpr auto operator()(Rng&& rng) const {
        return std::views::transform(std::forward<Rng>(rng), [](auto& element) { return &element; });
    }
};

inline cxpr to_ptr_range_fn to_ptr_range;


struct split_into_fn {
    /**
     * 
     * @param rng to chunk
     * @param max_chunks count of chunks to split into
     * @pre chunks > 0
     * @post split into `N` chunks where `N <= chunks` 
     * @return view of 
     */
    template<std::ranges::sized_range Rng>
    cxpr auto operator()(Rng&& rng, std::ranges::range_difference_t<Rng> const max_chunks) const {
        assert(max_chunks > 0 && "cannot chunk into <= 0 chunks");

        auto const rngSize = std::ranges::size(rng);
        auto const chunkSize = (rngSize + max_chunks - 1) / max_chunks;

        return std::views::chunk(std::forward<Rng>(rng), chunkSize);
    }

    template<class Fn, class T> requires std::constructible_from<std::decay_t<T>, T>
    cxpr auto operator()(this Fn&&, T&& chunks) {
        return ranges::pipe_call_closure<std::remove_cvref_t<Fn>, std::decay_t<T>>{std::forward<T>(chunks)};
    }
};

/**
 * splits a range into <= n chunks
 */
inline cxpr split_into_fn split_into;


struct drop_stride_fn {
    template<std::ranges::viewable_range Rng>
    cxpr auto operator()(
        Rng&& rng,
        std::ranges::range_difference_t<Rng> const drop,
        std::ranges::range_difference_t<Rng> const stride
    ) const {
        return std::views::stride(std::views::drop(std::forward<Rng>(rng), drop), stride);
    }

    template<class Fn, class T, class U> requires(std::constructible_from<std::decay_t<T>, T> && std::constructible_from<std::decay_t<U>, U>)
    cxpr auto operator()(this Fn&&, T&& drop, U&& stride) {
        return ranges::pipe_call_closure<std::remove_cvref_t<Fn>, std::decay_t<T>, std::decay_t<U>>{
            std::forward<T>(drop),
            std::forward<U>(stride)
        };
    }
};

inline cxpr drop_stride_fn drop_stride;


struct transform_call_fn {
    template<class Rng>
    using range_value_t = std::ranges::range_value_t<Rng>;

    template<std::ranges::range Rng, class Call, class... Args> requires(
        std::invocable<Call, std::ranges::range_value_t<Rng>, Args...>)
    cxpr auto operator()(Rng&& rng, Call&& call, Args&&... args) const {
        return std::views::transform(
            std::forward<Rng>(rng),
            [&call, &args...]<class T>(T&& element) {
                return std::invoke(
                    std::forward<Call>(call),
                    std::forward<T>(element),
                    std::forward<Args>(args)...
                );
            }
        );
    }

    template<class Call, class... Args, class Fn> requires(!std::ranges::range<Call>)
    cxpr auto operator()(this Fn&&, Call&& call, Args&&... args) {
        return ranges::pipe_call_closure<std::remove_cvref_t<Fn>, std::decay_t<Call>, std::decay_t<Args>...>{
            std::forward<Call>(call),
            std::forward<Args>(args)...
        };
    }
};

inline cxpr transform_call_fn transform_call;
}


namespace cth::views {
template<std::ranges::input_range Rng>
struct format_view {
    Rng rng;
    std::string_view sep;
    std::string_view pre;
    std::string_view post;
};

struct format_fn {

    /**
     * Formats a range with custom separator and pre / postfix.
     * @param rng to format
     * @param sep separator between elements
     * @param pre prefix
     * @param post postfix
     * @return formattable object
     */
    template<std::ranges::viewable_range Rng>
    [[nodiscard]] cxpr auto operator()(
        Rng&& rng,
        std::string_view sep,
        std::string_view pre = "",
        std::string_view post = ""
    ) const {
        return format_view{std::views::all(std::forward<Rng>(rng)), sep, pre, post};
    }

    template<class Me>
    [[nodiscard]] cxpr auto operator()(
        this Me&&,
        std::string_view sep,
        std::string_view pre = "",
        std::string_view post = ""
    ) {
        return ranges::pipe_call_closure
        <std::remove_cvref_t<Me>,
            std::string_view,
            std::string_view,
            std::string_view
        >{
            sep,
            pre,
            post
        };
    }

};

inline cxpr format_fn format;
}

template<class Rng>
struct std::formatter<cth::views::format_view<Rng>> {
    using value_type = std::ranges::range_value_t<Rng>;


    [[nodiscard]] cxpr auto parse(std::format_parse_context& ctx) {
        return _formatter.parse(ctx);
    }

    template<typename FormatContext>
    [[nodiscard]] cxpr auto format(
        cth::views::format_view<Rng>& obj,
        FormatContext& ctx
    ) const {

        auto& [rng, sep, pre, post] = obj;

        _formatter.set_separator(sep);
        _formatter.set_brackets(pre, post);

        return _formatter.format(rng, ctx);
    }

private:
    mutable std::range_formatter<value_type> _formatter{};

};
