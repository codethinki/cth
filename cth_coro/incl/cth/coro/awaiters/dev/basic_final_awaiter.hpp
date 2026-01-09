#pragma once
#include <coroutine>

namespace cth::co::dev {
struct basic_final_awaiter {
    constexpr bool await_ready() noexcept { return false; }

    template<class Promise>
    [[nodiscard]] constexpr auto await_suspend(std::coroutine_handle<Promise> h) noexcept -> std::coroutine_handle<> {
        if(auto continuation = h.promise().continuation) return continuation;
        return std::noop_coroutine();
    }

    constexpr void await_resume() noexcept {}
};
}
