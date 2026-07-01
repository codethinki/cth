#pragma once
#include "cth/coro/scheduler.hpp"
#include "cth/coro/this_coro/this_coro_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <coroutine>


namespace cth::co {
/**
 * switches execution onto a target @ref scheduler and stays there
 * @details 
 * @tparam Pyld payload of the migrating coroutine, defaulted to @ref this_coro::default_payload
 */
template<this_coro::scheduler_payload Pyld>
struct schedule_awaiter : this_coro_base<Pyld> {
public:
    constexpr schedule_awaiter(co::scheduler const& target) : _target{target} {}

    [[nodiscard]] bool await_ready() const noexcept { return _target.owns_thread(); }

    template<promise Promise> requires this_coro::scheduler_payload<Pyld>
    void await_suspend(std::coroutine_handle<Promise> h) {
        // even though we don't inject it, payload compatibility should be enforced
        static_assert(!this_coro_compatible<Promise> || this_coro::promise_injectable_with<Promise, Pyld>);

        if constexpr(this_coro::scheduler_payload<Pyld> && this_coro_compatible<Promise>)
            h.promise().rebind_scheduler(_target);

        _target.post([h]() { h.resume(); });
    }

    template<pyld_injectable_to<Pyld> P>
    constexpr void inject(P const&) const noexcept {}

    constexpr void await_resume() const noexcept {}

private:
    co::scheduler const& _target;
};
}
