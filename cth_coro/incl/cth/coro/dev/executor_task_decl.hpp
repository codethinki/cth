#pragma once
#include "base/executor_awaiter_base.hpp"
#include "base/dev/executor_promise_base_decl.hpp"

#include "dev/basic_promise.hpp"
#include "dev/unique_handle.hpp"

#include <coroutine>
#include <exception>
#include <type_traits>
#include <utility>

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
class [[nodiscard]] executor_task {
public:
    using promise_type = dev::executor_promise<T>;

    struct awaiter : executor_awaiter_base {

        awaiter(std::coroutine_handle<promise_type> h) : handle{h} {}


        [[nodiscard]] bool await_ready() const noexcept { return !handle || handle.done(); }

        template<class Promise>
        auto await_suspend(std::coroutine_handle<Promise> caller) noexcept -> std::coroutine_handle<promise_type> {
            executor_awaiter_base::inject_executor(caller);
            handle.promise().continuation = caller;
            return handle;
        }

        auto await_resume() {
            if(handle.promise().exception)
                std::rethrow_exception(handle.promise().exception);
            if constexpr(!std::same_as<void, T>)
                return std::move(*handle.promise().result);
        }

        std::coroutine_handle<promise_type> handle;
    };

    ~executor_task() = default;

private:
    friend struct promise_type;
    explicit executor_task(std::coroutine_handle<promise_type> h) noexcept : _handle{h} {}

    dev::unique_handle<promise_type> _handle;

public:
    auto handle(this auto&& self) { return self._handle.get(); }

    auto operator co_await() const & noexcept { return awaiter{handle()}; }
    auto operator co_await() && noexcept { return awaiter{this->_handle.extract()}; }

    executor_task(executor_task const&) = delete;
    executor_task operator=(executor_task const&) = delete;
    executor_task(executor_task&&) noexcept = default;
    executor_task& operator=(executor_task&&) noexcept = default;
};


template<class T>
auto dev::executor_promise<T>::get_return_object() noexcept -> executor_task<T> {
    return executor_task<T>{std::coroutine_handle<executor_promise<T>>::from_promise(*this)};
}

using executor_void_task = executor_task<void>;

}


