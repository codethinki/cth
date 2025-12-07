#pragma once
#include "cth/coro/awaiters/dev/capture_awaiter_base.hpp"
#include "cth/coro/tasks/promises/basic_promise.hpp"
#include "cth/coro/tasks/promises/executor_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/coro/unique_cohandle.hpp>

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
class [[nodiscard]] scheduled_task {

public:
    using promise_type = dev::scheduled_promise<T>;

    struct awaiter : dev::capture_awaiter_base {
        std::coroutine_handle<promise_type> handle;

        [[nodiscard]] bool await_ready() const noexcept { return !handle || handle.done(); }

        auto await_suspend(std::coroutine_handle<> caller) noexcept -> std::coroutine_handle<> {
            handle.promise().continuation = caller;
            return handle;
        }

        auto await_resume() {
            if(handle.promise().exception)
                std::rethrow_exception(handle.promise().exception);
            if constexpr(!std::is_void_v<T>)
                return std::move(*handle.promise().result);
        }
    };

    ~scheduled_task() = default;

private:
    friend struct promise_type;
    explicit scheduled_task(std::coroutine_handle<promise_type> h) noexcept : _handle{h} {}
    dev::unique_cohandle<promise_type> _handle;

public:
    auto handle(this auto&& self) { return self._handle.get(); }

    auto operator co_await() const & noexcept { return awaiter{handle()}; }
    auto operator co_await() && noexcept { return awaiter{_handle->extract()}; }

    scheduled_task(scheduled_task&&) noexcept = default;
    scheduled_task& operator=(scheduled_task&&) noexcept = default;
};

template<class T>
scheduled_task<T> dev::scheduled_promise<T>::get_return_object() noexcept {
    return scheduled_task<T>{std::coroutine_handle<scheduled_promise>::from_promise(*this)};
}

}
