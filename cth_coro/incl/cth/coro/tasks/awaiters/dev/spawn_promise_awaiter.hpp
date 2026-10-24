#pragma once
#include "basic_promise_awaiter.hpp"

#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/this_coro/this_coro_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <coroutine>


namespace cth::co::dev {
/**
 * awaiter of @ref spawn_task, takes the context of the caller from the injected payload
 * @details the payload of the caller is not stored and not rebound,
 *  a caller without this_coro support injects nothing
 * @tparam Promise of the spawned coroutine, needs a `caller` member
 */
template<cth_promise Promise>
struct spawn_promise_awaiter : this_coro_base<this_coro::default_payload>, private basic_promise_awaiter<Promise> {
private:
    using base_t = basic_promise_awaiter<Promise>;

public:
    using promise_type = base_t::promise_type;

    using base_t::await_ready;
    using base_t::await_resume;
    using base_t::base_t;

    /**
     * stores the context of the caller
     * @param payload of the caller
     */
    template<this_coro::context_payload Pyld>
    constexpr void inject(Pyld const& payload) noexcept { _caller = &payload.context(); }

    [[nodiscard]] auto await_suspend(std::coroutine_handle<> caller) noexcept -> std::coroutine_handle<promise_type> {
        base_t::handle.promise().caller = _caller;
        return base_t::await_suspend(caller);
    }

private:
    co::context const* _caller = nullptr;
};
}
