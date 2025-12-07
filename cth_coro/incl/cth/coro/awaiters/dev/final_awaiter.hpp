#pragma once
#include <coroutine>

namespace cth::co::dev {
struct final_awaiter {
    bool await_ready() noexcept { return false; }

    template<class Promise>
    auto await_suspend(std::coroutine_handle<Promise> h) noexcept -> std::coroutine_handle<> {
        if(auto continuation = h.promise().continuation) return continuation;
        return std::noop_coroutine();
    }

    void await_resume() noexcept {}
};
}
