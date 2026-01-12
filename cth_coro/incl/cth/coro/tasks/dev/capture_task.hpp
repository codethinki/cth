#pragma once
#include "cth/coro/tasks/task_base.hpp"

#include "cth/coro/tasks/awaiters/dev/capture_promise_awaiter.hpp"
#include "cth/coro/tasks/promises/basic_promise.hpp"

#include <coroutine>

namespace cth::co {

namespace dev {
    template<class T>
    struct capture_promise : basic_promise<T> {
        capture_task<T> get_return_object() noexcept;
    };
}


template<class T>
class [[nodiscard]] capture_task : public task_base<dev::capture_promise<T>, dev::capture_promise_awaiter> {
    using base_t = task_base<dev::capture_promise<T>, dev::capture_promise_awaiter>;

    using base_t::base_t;
    friend base_t::promise_type;
};

namespace dev {
    template<class T>
    capture_task<T> capture_promise<T>::get_return_object() noexcept {
        return capture_task<T>{std::coroutine_handle<capture_promise>::from_promise(*this)};
    }
}

}
