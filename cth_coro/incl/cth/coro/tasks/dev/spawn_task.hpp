#pragma once
#include "cth/coro/tasks/task_base.hpp"

#include "cth/coro/tasks/awaiters/dev/spawn_final_awaiter.hpp"
#include "cth/coro/tasks/awaiters/dev/spawn_promise_awaiter.hpp"
#include "cth/coro/tasks/promises/basic_promise.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <coroutine>


namespace cth::co::dev {

template<class T>
struct spawn_promise : basic_promise<T, std::suspend_always{}, spawn_final_awaiter{}> {
    /**
     * context of the awaiting coroutine, nullptr -> unknown
     */
    co::context const* caller = nullptr;

    spawn_task<T> get_return_object() noexcept;
};

}

namespace cth::co {

/**
 * task of @ref executor::spawn()
 * @details this_coro awaitable: the caller continues on its own context after the spawned coroutine ended,
 *  a caller without this_coro support continues where the spawned coroutine ended
 * @tparam T result type
 */
template<class T>
class [[nodiscard]] spawn_task : public task_base<dev::spawn_promise<T>, dev::spawn_promise_awaiter> {
    using base_t = task_base<dev::spawn_promise<T>, dev::spawn_promise_awaiter>;

    using base_t::base_t;
    friend base_t::promise_type;

public:
    spawn_task(spawn_task const& other) = delete;
    spawn_task& operator=(spawn_task const& other) = delete;

    spawn_task(spawn_task&& other) noexcept = default;
    spawn_task& operator=(spawn_task&& other) noexcept = default;
};

template<class T>
spawn_task<T> dev::spawn_promise<T>::get_return_object() noexcept {
    return spawn_task<T>{std::coroutine_handle<spawn_promise>::from_promise(*this)};
}

}
