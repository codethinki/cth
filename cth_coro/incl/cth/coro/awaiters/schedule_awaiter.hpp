#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/this_coro/this_coro_base.hpp"
#include "cth/coro/utility/concepts.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <coroutine>


namespace cth::co {
/**
 * switches execution onto a target @ref context and stays there
 * @details noop if the coroutine already runs on the target: @ref context::current() is the target
 * @tparam Pyld payload of the migrating coroutine, defaulted to @ref this_coro::default_payload
 */
template<this_coro::context_payload Pyld>
struct schedule_awaiter : this_coro_base<Pyld> {
public:
    constexpr schedule_awaiter(co::context const& target) : _target{target} {}

    /**
     * @return false, @ref await_suspend() decides
     */
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

    /**
     * rebinds the payload to the target and posts the coroutine onto the target
     * @details owns_thread() is not enough: a scheduler also owns the threads that run its strands
     * @return false if the coroutine already runs on the target
     */
    template<promise Promise> requires this_coro::context_payload<Pyld>
    bool await_suspend(std::coroutine_handle<Promise> h) {
        // even though we don't inject it, payload compatibility should be enforced
        static_assert(!this_coro_compatible<Promise> || this_coro::promise_injectable_with<Promise, Pyld>);

        if constexpr(has_context<Promise>)
            h.promise().rebind_context(_target);

        if(context::current() == &_target)
            return false;

        _target.post([h]() { h.resume(); });
        return true;
    }

    template<pyld_injectable_to<Pyld> P>
    constexpr void inject(P const&) const noexcept {}

    constexpr void await_resume() const noexcept {}

private:
    co::context const& _target;
};
}
