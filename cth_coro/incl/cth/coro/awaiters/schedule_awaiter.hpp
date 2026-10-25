#pragma once
#include "cth/coro/scheduler.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <coroutine>


namespace cth::co {
/**
 * switches execution onto a target @ref scheduler and stays there
 * @details a this_coro caller runs on the target afterwards, its payload stays
 */
struct schedule_awaiter : this_coro_awaiter_base {
public:
    constexpr schedule_awaiter(co::scheduler const& target) : _target{target} {}

    [[nodiscard]] bool await_ready() const noexcept { return _target.owns_thread(); }

    template<promise Promise>
    void await_suspend(std::coroutine_handle<Promise> h) {
        if constexpr(this_coro_compatible<Promise>)
            h.promise().rebind_scheduler(_target);

        _target.post([h]() { h.resume(); });
    }

    constexpr void await_resume() const noexcept {}

private:
    co::scheduler const& _target;
};
}
