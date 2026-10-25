#pragma once
#include "cth/coro/tasks/task_base.hpp"
#include "cth/coro/tasks/awaiters/this_coro_promise_awaiter.hpp"
#include "cth/coro/tasks/promises/affine_promise.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"

#include "cth/coro/utility/fwd.hpp"

#include <coroutine>


namespace cth::co::dev {
template<class T, payload Pyld = this_coro::default_payload>
struct executor_promise : dev::affine_promise<T>, this_coro_promise_base<Pyld> {
    executor_task<T, Pyld> get_return_object() noexcept;
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
