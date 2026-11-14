#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <coroutine>


namespace cth::co {
/**
 * awaiter of `strand::operator co_await()`
 */
struct strand_wait_awaiter : this_coro_awaiter_base {
    co::context const& strand;

    [[nodiscard]] static constexpr bool await_ready() noexcept { return false; }

    template<this_coro_compatible Caller>
    void await_suspend(std::coroutine_handle<Caller> caller) const {
        // behind every job posted before, then on the context of the caller: inline if it is the strand
        strand.post([caller, &target = caller.promise().context()] { dev::transfer_on(target, caller).resume(); });
    }

    static constexpr void await_resume() noexcept {}
};
}
