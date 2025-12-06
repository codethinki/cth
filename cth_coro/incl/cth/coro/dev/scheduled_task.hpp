#pragma once
#include "basic_promise.hpp"
#include "unique_handle.hpp"
#include "cth/coro/base/dev/capture_awaiter_base.hpp"
#include "cth/coro/base/dev/executor_promise_base_decl.hpp"

namespace cth::co {
class executor;
template<class T> class scheduled_task;
}

namespace cth::co::dev {

template<class T>
struct scheduled_promise : basic_promise<T>, executor_promise_base {
    scheduled_promise(executor& e) : executor_promise_base{&e} {}
    scheduled_promise() = default;

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
    dev::unique_handle<promise_type> _handle;

public:
    auto operator co_await() const & noexcept { return awaiter{this->_handle}; }
    auto operator co_await() && noexcept { return awaiter{this->extract()}; }

    scheduled_task(scheduled_task&&) noexcept = default;
    scheduled_task& operator=(scheduled_task&&) noexcept = default;
};

namespace dev {
    template<class T>
    scheduled_task<T> scheduled_promise<T>::get_return_object() noexcept {
        return scheduled_task<T>{std::coroutine_handle<scheduled_promise>::from_promise(*this)};
    }
}

}
