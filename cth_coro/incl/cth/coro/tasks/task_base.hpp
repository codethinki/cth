#pragma once
#include "cth/coro/utility/concepts.hpp"

#include <cth/coro/unique_cohandle.hpp>

#include <coroutine>

namespace cth::co {
template<cth_promise Promise, template<class> class Awaiter>
class [[nodiscard]] task_base {
public:
    using promise_type = Promise;
    using awaiter_type = Awaiter<Promise>;



    ~task_base() = default;

protected:
    task_base(std::coroutine_handle<promise_type> h) noexcept : _handle{h} {}

private:
    unique_cohandle<promise_type> _handle;

public:
    auto handle(this auto&& self) { return self._handle.get(); }

    auto operator co_await() const & noexcept { return awaiter_type{handle()}; }
    auto operator co_await() && noexcept { return awaiter_type{this->_handle.extract()}; }

    task_base(task_base const&) = delete;
    task_base operator=(task_base const&) = delete;
    task_base(task_base&&) noexcept = default;
    task_base& operator=(task_base&&) noexcept = default;
};

}
