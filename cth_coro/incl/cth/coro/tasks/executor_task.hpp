#pragma once
#include "cth/coro/awaiters/executor_awaiter.hpp"
#include "promises/basic_promise.hpp"
#include "promises/executor_promise_base.hpp"

#include "cth/coro/tasks/task_base.hpp"

#include <coroutine>

namespace cth::co {
template<class T>
class executor_task;
}

namespace cth::co::dev {
template<class T>
struct executor_promise : basic_promise<T>, executor_promise_base {
    executor_task<T> get_return_object() noexcept;
};
}


namespace cth::co {
template<class T>
class [[nodiscard]] executor_task : public task_base<dev::executor_promise<T>, executor_awaiter> {
    using base_t = task_base<dev::executor_promise<T>, executor_awaiter>;
    using base_t::task_base;

    friend base_t::promise_type;
};


template<class T>
auto dev::executor_promise<T>::get_return_object() noexcept -> executor_task<T> {
    return executor_task<T>{std::coroutine_handle<executor_promise<T>>::from_promise(*this)};
}

using executor_void_task = executor_task<void>;

}
