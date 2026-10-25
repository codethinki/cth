#pragma once
#include "cth/coro/tasks/task_base.hpp"

#include "cth/coro/tasks/awaiters/dev/affine_final_promise_awaiter.hpp"
#include "cth/coro/tasks/awaiters/dev/spawn_promise_awaiter.hpp"
#include "cth/coro/tasks/promises/affine_promise_base.hpp"
#include "cth/coro/tasks/promises/basic_promise.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <coroutine>


namespace cth::co::dev {

template<class T, payload Pyld = this_coro::default_payload>
struct spawn_promise
    : basic_promise<T, std::suspend_always{}, dev::affine_final_promise_awaiter{}>
    , dev::affine_promise_base
    , this_coro_promise_base<Pyld> {
    /**
     * constructs from the coroutine parameters, runs on the scheduler of the executor
     * @details defined in executor.hpp, the executor is incomplete here
     * @param payload of the spawned task
     * @param self executor that spawns the task
     */
    template<class... Args>
    explicit spawn_promise(Pyld const& payload, executor const& self, Args&&...);


    spawn_task<T, Pyld> get_return_object() noexcept;
};

}

namespace cth::co {

/**
 * task of @ref executor::spawn()
 * @details runs on the scheduler of the executor with its own payload,
 *  the caller continues on its scheduler, a caller without a scheduler continues on the scheduler of the executor
 * @tparam T result type
 * @tparam Pyld payload of the spawned task
 */
template<class T, payload Pyld>
class [[nodiscard]] spawn_task
    : public task_base<dev::spawn_promise<T, Pyld>, dev::spawn_promise_awaiter> {
    using base_t = task_base<dev::spawn_promise<T, Pyld>, dev::spawn_promise_awaiter>;

    using base_t::base_t;
    friend base_t::promise_type;

public:
    spawn_task(spawn_task const& other) = delete;
    spawn_task& operator=(spawn_task const& other) = delete;

    spawn_task(spawn_task&& other) noexcept = default;
    spawn_task& operator=(spawn_task&& other) noexcept = default;
};

template<class T, payload Pyld>
spawn_task<T, Pyld> dev::spawn_promise<T, Pyld>::get_return_object() noexcept {
    return spawn_task<T, Pyld>{std::coroutine_handle<spawn_promise>::from_promise(*this)};
}

}
