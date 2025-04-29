#pragma once
#include "../macro.hpp"

#include <concepts>
#include <tuple>

namespace cth::ranges {
class piped_fn {};
template<class T, class Fn> requires std::derived_from<std::remove_cvref_t<Fn>, piped_fn>
cxpr dclauto operator|(T&& left, Fn&& fn) { return std::forward<Fn>(fn)(std::forward<T>(left)); }
}

namespace cth::ranges {
template<class Fn, class... FnArgs>
struct pipe_call_closure : piped_fn {
    static_assert(std::same_as<std::decay_t<FnArgs>, FnArgs>, "the contained types must not be references");

    template<class... CArgs> requires(std::same_as<std::decay_t<CArgs>, FnArgs> && ...)
    explicit cxpr pipe_call_closure(CArgs&&... args) : _args{std::forward<CArgs>(args)...} {}

    template<class T, class Me>
    cxpr dclauto operator()(this Me&& me, T&& arg) {
        return std::apply([&arg]<class... Args>(this auto&&, Args&&... args) -> dclauto {
            return Fn{}.template operator()<T>(std::forward<T>(arg), std::forward<Args>(args)...);
        }, std::forward<Me>(me)._args);
    }

private:
    std::tuple<FnArgs...> _args;
};

}