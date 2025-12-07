#pragma once
#include "cth/coro/awaiters/executor_awaiter.hpp"
#include "promises/basic_promise.hpp"
#include "promises/executor_promise_base.hpp"


#include <cth/coro/unique_cohandle.hpp>

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
class [[nodiscard]] executor_task {
public:
    using promise_type = dev::executor_promise<T>;
    using awaiter = executor_awaiter<promise_type>;


    ~executor_task() = default;

private:
    friend struct promise_type;
    explicit executor_task(std::coroutine_handle<promise_type> h) noexcept : _handle{h} {}

    dev::unique_cohandle<promise_type> _handle;

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
