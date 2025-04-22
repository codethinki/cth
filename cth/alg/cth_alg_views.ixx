module;

#include "../macro.hpp"

#include "../io/io_log.hpp"

export module cth.alg.views;

import cth.io.log;

import cth.alg.utility;
import std;

export namespace cth::views {
struct to_ptr_range_fn : ranges::piped_fn {
    template<std::ranges::viewable_range Rng> requires std::is_lvalue_reference_v<std::ranges::range_reference_t<Rng>>
    cxpr auto operator()(Rng&& rng) const { return std::views::transform(std::forward<Rng>(rng), [](auto& element) { return &element; }); }
};

cxpr to_ptr_range_fn to_ptr_range;



struct split_into_fn {
    template<std::ranges::sized_range Rng>
    cxpr auto operator()(Rng&& rng, std::ranges::range_difference_t<Rng> const chunks) const {
        CTH_CRITICAL(chunks <= 0, "requires chunks to be > 0, current: []", chunks) {}
        auto const rngSize = std::ranges::size(rng);
        auto const chunkSize = rngSize / chunks + (rngSize % chunks == 0 ? 0 : 1);
        return std::views::chunk(std::forward<Rng>(rng), chunkSize);
    }

    template<class Fn, class T> requires std::constructible_from<std::decay_t<T>, T>
    cxpr auto operator()(this Fn&&, T&& chunks) {
        return ranges::pipe_call_closure<std::remove_cvref_t<Fn>, std::decay_t<T>>{std::forward<T>(chunks)};
    }

};

cxpr split_into_fn split_into;



struct drop_stride_fn {
    template<std::ranges::range Rng>
    cxpr auto operator()(Rng&& rng, std::ranges::range_difference_t<Rng> const drop, std::ranges::range_difference_t<Rng> const stride) const {
        return std::views::stride(std::views::drop(std::forward<Rng>(rng), drop), stride);
    }

    template<class Fn, class T, class U> requires (std::constructible_from<std::decay_t<T>, T> && std::constructible_from<std::decay_t<U>, U>)
    cxpr auto operator()(this Fn&&, T&& drop, U&& stride) {
        return ranges::pipe_call_closure<std::remove_cvref_t<Fn>, std::decay_t<T>, std::decay_t<U>>{std::forward<T>(drop), std::forward<U>(stride)};
    }

};

cxpr drop_stride_fn drop_stride;



struct transform_call_fn {
    template<class Rng>
    using range_value_t = std::ranges::range_value_t<Rng>;

    template<std::ranges::range Rng, class Call, class... Args> requires(std::is_invocable_v<Call, std::ranges::range_value_t<Rng>, Args...>)
    cxpr auto operator()(Rng&& rng, Call&& call, Args&&... args) const {
        return std::views::transform(std::forward<Rng>(rng), [&call, &args...]<class T>(T&& element) {
            return std::invoke(std::forward<Call>(call), std::forward<T>(element), std::forward<Args>(args)...);
        });
    }

    template<class Call, class... Args, class Fn> requires(!std::ranges::range<Call>)
    cxpr auto operator()(this Fn&&, Call&& call, Args&&... args) {
        return ranges::pipe_call_closure<std::remove_cvref_t<Fn>, std::decay_t<Call>, std::decay_t<Args>...>{
            std::forward<Call>(call),
            std::forward<Args>(args)...
        };
    }


};

cxpr transform_call_fn transform_call;
}
