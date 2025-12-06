#pragma once
#include "basic_promise.hpp"
#include "unique_handle.hpp"

#include "../base/dev/capture_awaiter_base.hpp"

#include <coroutine>

namespace cth::co {

template<class T>
class capture_task;

namespace dev {
    template<class T>
    struct capture_promise : dev::basic_promise<T> {
        capture_task<T> get_return_object() noexcept;
    };
}


template<class T>
class [[nodiscard]] capture_task {
public:
    using promise_type = dev::capture_promise<T>;

    struct awaiter : dev::capture_awaiter_base {
        awaiter(std::coroutine_handle<promise_type> h) : handle{h} {}

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

    ~capture_task() = default;

private:
    friend struct promise_type;
    explicit capture_task(std::coroutine_handle<promise_type> h) noexcept : _handle{h} {}

    dev::unique_handle<promise_type> _handle;

public:
    auto handle(this auto&& self) { return self._handle.get(); }

    awaiter operator co_await() const & noexcept { return {handle()}; }
    awaiter operator co_await() && noexcept { return {_handle.extract()}; }

    capture_task(capture_task&&) noexcept = default;
    capture_task& operator=(capture_task&&) noexcept = default;
};

namespace dev {
    template<class T>
    capture_task<T> capture_promise<T>::get_return_object() noexcept {
        return capture_task<T>{std::coroutine_handle<capture_promise>::from_promise(*this)};
    }
}

}
