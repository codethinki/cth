#pragma once
#include "cth/coro/tasks/task_base.hpp"

#include "cth/coro/tasks/awaiters/dev/capture_promise_awaiter.hpp"
#include "cth/coro/tasks/promises/basic_promise.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::dev {

template<class T>
struct scheduled_promise
    : basic_promise<T>
    , this_coro_promise_base {
    explicit scheduled_promise(this_coro::payload payload) : this_coro_promise_base{std::move(payload)} {}

    template<class... Args>
    explicit scheduled_promise(this_coro::payload const& payload, Args&&...) : scheduled_promise{payload} {}


    scheduled_task<T> get_return_object() noexcept;
};

}

namespace cth::co {

template<class T>
class [[nodiscard]] scheduled_task
    : public task_base<dev::scheduled_promise<T>, dev::capture_promise_awaiter> {
    using base_t = task_base<dev::scheduled_promise<T>, dev::capture_promise_awaiter>;

    using base_t::base_t;
    friend base_t::promise_type;

public:
    scheduled_task(scheduled_task const& other) = delete;
    scheduled_task& operator=(scheduled_task const& other) = delete;

    scheduled_task(scheduled_task&& other) noexcept = default;
    scheduled_task& operator=(scheduled_task&& other) noexcept = default;
};

template<class T>
scheduled_task<T> dev::scheduled_promise<T>::get_return_object() noexcept {
    return scheduled_task<T>{std::coroutine_handle<scheduled_promise>::from_promise(*this)};
}

}
