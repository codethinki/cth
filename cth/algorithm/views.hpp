#pragma once
#include "../io/log.hpp"

#include <ranges>


namespace cth::views {
class piped_fn {};
template<class T, class Fn> requires std::derived_from<std::remove_cvref_t<Fn>, piped_fn>
constexpr decltype(auto) operator|(T&& left, Fn&& fn) { return std::forward<Fn>(fn)(std::forward<T>(left)); }
}

namespace cth::views {
template<class Fn, class... FnArgs>
struct pipe_call_closure : piped_fn {
    static_assert(std::same_as<std::decay_t<FnArgs>, FnArgs>, "the contained types must not be references");

    template<class... CArgs> requires(std::same_as<std::decay_t<CArgs>, FnArgs> && ...)
    explicit constexpr pipe_call_closure(CArgs&&... args) : _args{std::forward<CArgs>(args)...} {}

    template<class T>
    constexpr decltype(auto) operator()(T&& arg) {
        return std::apply([&arg]<class... TplArgs>(TplArgs&&... args) mutable -> decltype(auto) {
            return Fn{}(std::forward<T>(arg), std::forward<TplArgs>(args)...);
        }, _args);
    }

private:
    std::tuple<FnArgs...> _args;
};

}

namespace cth::views {
struct to_ptr_range_fn : piped_fn {
    template<std::ranges::viewable_range Rng> requires std::is_lvalue_reference_v<std::ranges::range_reference_t<Rng>>
    constexpr auto operator()(Rng&& rng) const { return std::views::transform(std::forward<Rng>(rng), [](auto& element) { return &element; }); }
};

inline constexpr to_ptr_range_fn to_ptr_range;



struct split_into_fn {
    template<std::ranges::viewable_range Rng> requires std::ranges::sized_range<Rng>
    constexpr auto operator()(Rng&& rng, std::ranges::range_difference_t<Rng> const chunks) const {
        CTH_CRITICAL(chunks <= 0, "requires chunks to be > 0, current: []", chunks) {}
        auto const rngSize = std::ranges::size(rng);
        auto const chunkSize = rngSize / chunks + (rngSize % chunks == 0 ? 0 : 1);
        return std::views::chunk(std::forward<Rng>(rng), chunkSize);
    }

    template<class Fn, class T> requires std::constructible_from<std::decay_t<T>, T>
    constexpr auto operator()(this Fn&&, T&& chunks) { return pipe_call_closure<std::remove_cvref_t<Fn>, std::decay_t<T>>{std::forward<T>(chunks)}; }

};

inline constexpr split_into_fn split_into;



struct drop_stride_fn {
    template<std::ranges::viewable_range Rng> requires std::ranges::sized_range<Rng>
    constexpr auto operator()(Rng&& rng, std::ranges::range_difference_t<Rng> const stride, std::ranges::range_difference_t<Rng> const drop_size) const {
        return std::views::stride(std::views::drop(std::forward<Rng>(rng), drop_size), stride);
    }
    template<std::ranges::viewable_range Rng> requires std::ranges::sized_range<Rng>
    constexpr auto operator()(Rng&& rng, std::ranges::range_difference_t<Rng> const stride_drop) const {
        return operator()(std::forward<Rng>(rng), stride_drop, stride_drop);
    }

    template<class Fn, class T> requires std::constructible_from<std::decay_t<T>, T>
    constexpr auto operator()(this Fn&&, T&& stride_drop) {
        return pipe_call_closure<std::remove_cvref_t<Fn>, std::decay_t<T>>{std::forward<T>(stride_drop)};
    }
    template<class Fn, class T, class U> requires std::constructible_from<std::decay_t<T>, T> && std::constructible_from<std::decay_t<U>, U>
    constexpr auto operator()(this Fn&&, T&& stride, U&& drop_size) {
        return pipe_call_closure<std::remove_cvref_t<Fn>, std::decay_t<T>, std::decay_t<U>>{std::forward<T>(stride), std::forward<U>(drop_size)};
    }
};

inline constexpr drop_stride_fn drop_stride;
}
