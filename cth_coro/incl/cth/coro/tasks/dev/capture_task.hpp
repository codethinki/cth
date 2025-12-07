#pragma once
#include "cth/coro/unique_cohandle.hpp"
#include "cth/coro/awaiters/dev/capture_awaiter.hpp"
#include "cth/coro/tasks/promises/basic_promise.hpp"
#include "cth/coro/utility/fwd.hpp"


#include <coroutine>

namespace cth::co {

namespace dev {
    template<class T>
    struct capture_promise : basic_promise<T> {
        capture_task<T> get_return_object() noexcept;
    };
}


template<class T>
class [[nodiscard]] capture_task {
public:
    using promise_type = dev::capture_promise<T>;
    using awaiter_type = dev::capture_awaiter<promise_type>;

    ~capture_task() = default;

private:
    friend struct promise_type;
    explicit capture_task(std::coroutine_handle<promise_type> h) noexcept : _handle{h} {}

    dev::unique_cohandle<promise_type> _handle;

public:
    auto handle(this auto&& self) { return self._handle.get(); }

    awaiter_type operator co_await() const & noexcept { return {handle()}; }
    awaiter_type operator co_await() && noexcept { return {_handle.extract()}; }

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
