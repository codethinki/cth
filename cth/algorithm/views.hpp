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
    explicit constexpr pipe_call_closure(FnArgs&&... args) : _args{std::forward<FnArgs>(args)...} {}
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
}

namespace cth::views {
struct split_into_fn {
    template<std::ranges::viewable_range Rng> requires std::ranges::sized_range<Rng>
    constexpr auto operator()(Rng&& rng, std::convertible_to<std::ranges::range_difference_t<Rng>> auto chunks) const {
        CTH_CRITICAL(chunks <= 0, "requires chunks to be > 0, current: []", chunks) {}
        auto const rngSize = std::ranges::size(rng);
        auto const chunkSize = rngSize / chunks + (rngSize % chunks == 0 ? 0 : 1);
        return std::views::chunk(std::forward<Rng>(rng), chunkSize);
    }

    template<class Fn, class T>
    constexpr auto operator()(this Fn&&, T&& chunks) { return pipe_call_closure<std::remove_cvref_t<Fn>, T>{std::forward<T>(chunks)}; }

};

inline constexpr split_into_fn split_into;
}
