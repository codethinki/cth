#pragma once
#include "cth/coro/tasks/task_base.hpp"

#include "cth/coro/tasks/awaiters/this_coro_promise_awaiter.hpp"
#include "cth/coro/tasks/promises/affine_promise.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <coroutine>


namespace cth::co::dev {

/**
 * promise of `spawn_task`
 */
template<class T, payload Pyld = this_coro::default_payload>
struct spawn_promise : dev::affine_promise<T>, this_coro_promise_base<Pyld> {
    /**
     * constructs from the coroutine parameters
     * @param payload of the spawned task
     * @param context the task runs on
     */
    template<class... Args>
    constexpr explicit spawn_promise(Pyld const& payload, co::context const& context, Args&&...) :
        this_coro_promise_base<Pyld>{context, payload} {}


    spawn_task<T, Pyld> get_return_object() noexcept;

    /**
     * stores the context the caller continues on, copies no payload
     * @details a caller without a context continues on the context of the task
     * @param caller promise of the caller
     */
    template<class CallerPromise>
    constexpr void attach(CallerPromise const& caller) noexcept {
        if constexpr(this_coro_compatible<CallerPromise>)
            this->callerContext = &caller.context();
        else
            this->callerContext = &this->context();
    }
};

}

namespace cth::co {

/**
 * task of `executor::spawn()`
 * @details runs on the context of the executor with its own payload
 * @details the caller continues on its context, a caller without a context continues on the context of the executor
 * @tparam T result type
 * @tparam Pyld payload of the spawned task
 */
template<class T, payload Pyld>
class [[nodiscard]] spawn_task
    : public task_base<dev::spawn_promise<T, Pyld>, this_coro_promise_awaiter> {
    using base_t = task_base<dev::spawn_promise<T, Pyld>, this_coro_promise_awaiter>;

    using base_t::base_t;
    friend base_t::promise_type;

public:
    spawn_task(spawn_task const& other) = delete;
    spawn_task& operator=(spawn_task const& other) = delete;

    constexpr spawn_task(spawn_task&& other) noexcept = default;
    constexpr spawn_task& operator=(spawn_task&& other) noexcept = default;
};

template<class T, payload Pyld>
spawn_task<T, Pyld> dev::spawn_promise<T, Pyld>::get_return_object() noexcept {
    return spawn_task<T, Pyld>{std::coroutine_handle<spawn_promise>::from_promise(*this)};
}

}
