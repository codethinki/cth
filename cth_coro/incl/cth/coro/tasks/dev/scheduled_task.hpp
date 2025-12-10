#pragma once
#include "cth/coro/tasks/task_base.hpp"

#include "cth/coro/awaiters/dev/capture_awaiter.hpp"
#include "cth/coro/tasks/promises/basic_promise.hpp"
#include "cth/coro/tasks/promises/executor_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::dev {

template<class T>
struct scheduled_promise : basic_promise<T>, executor_promise_base {
    template<class... Args>
    scheduled_promise(executor& e, Args&&...) : executor_promise_base{&e} {}

    scheduled_task<T> get_return_object() noexcept;
};

}

namespace cth::co {

template<class T>
class [[nodiscard]] scheduled_task : public task_base<dev::scheduled_promise<T>, dev::capture_awaiter> {
    using base_t = task_base<dev::scheduled_promise<T>, dev::capture_awaiter>;

    using base_t::task_base;
    friend base_t::promise_type;
};

template<class T>
scheduled_task<T> dev::scheduled_promise<T>::get_return_object() noexcept {
    return scheduled_task<T>{std::coroutine_handle<scheduled_promise>::from_promise(*this)};
}

}
