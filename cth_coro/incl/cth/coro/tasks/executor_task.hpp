#pragma once
#include "cth/coro/tasks/task_base.hpp"
#include "cth/coro/tasks/awaiters/this_coro_promise_awaiter.hpp"
#include "cth/coro/tasks/promises/affine_promise.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/this_coro/this_coro_base.hpp"

#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/concepts.hpp>
#include <cth/meta/traits.hpp>

#include <coroutine>


namespace cth::co::dev {
template<class T, payload Pyld = this_coro::default_payload>
struct executor_promise : dev::affine_promise<T>, this_coro_promise_base<Pyld> {
    executor_task<T, Pyld> get_return_object() noexcept;

    /**
     * copies the context and the payload of the caller, the task continues the caller on its context
     * @param caller promise of the awaiting coroutine
     */
    template<class CallerPromise>
    constexpr void attach(CallerPromise const& caller)
        noexcept(mta::nothrow_constructible<Pyld, caller_payload_t<CallerPromise> const&>) {
        static_assert(
            this_coro_compatible<CallerPromise>,
            "this_coro tasks need a this_coro caller, use spawn() or sync_wait()"
        );

        this->inherit(caller);
        dev::affine_promise<T>::attach(caller);
    }

private:
    /**
     * payload of the caller, the own payload for a caller without one (the static_assert in attach reports it)
     */
    template<class CallerPromise>
    using caller_payload_t = mta::conditional_pack_t<
        this_coro_compatible<CallerPromise>,
        mta::trait_pack<payload_t, CallerPromise>,
        mta::type_pack<Pyld>>;
};
}


namespace cth::co {
/**
 * Task to be spawned on the @ref executor
 * @details runs on the context of the caller, the caller continues on its context afterwards
 * @tparam T task return type
 * @tparam Pyld payload of the task, the payload of the caller must convert to it
 */
template<class T, payload Pyld>
class [[nodiscard]] executor_task : public task_base<dev::executor_promise<T, Pyld>, this_coro_promise_awaiter> {
    using base_t = task_base<dev::executor_promise<T, Pyld>, this_coro_promise_awaiter>;
    using base_t::base_t;

    friend base_t::promise_type;
};


template<class T, payload Pyld>
auto dev::executor_promise<T, Pyld>::get_return_object() noexcept -> executor_task<T, Pyld> {
    return executor_task<T, Pyld>{std::coroutine_handle<executor_promise<T, Pyld>>::from_promise(*this)};
}

using executor_void_task = executor_task<void>;

}
