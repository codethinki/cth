#pragma once
#include "cth/coro/utility/concepts.hpp"

namespace cth::co {
struct final_sync_awaiter {
    constexpr bool await_ready() noexcept { return false; }

    template<sync_promise_type Promise>
    auto await_suspend(std::coroutine_handle<Promise> h) noexcept -> std::coroutine_handle<> {
        auto co = h.promise().continuation;

        h.promise().signal();

        if(co)
            return co;
        return std::noop_coroutine();
    }

    constexpr void await_resume() noexcept {}
};

}
