#pragma once
#include "cth/coro/tasks/awaiters/dev/basic_promise_awaiter.hpp"
#include "cth/coro/tasks/awaiters/this_coro_promise_awaiter_base.hpp"

#include <cth/meta/coro.hpp>
#include <exception>

namespace cth::co {
template<cth_promise Promise>
struct this_coro_promise_awaiter
    : this_coro_promise_awaiter_base<typename Promise::payload_type>,
    private dev::basic_promise_awaiter<Promise> {
private:
    using base_t = dev::basic_promise_awaiter<Promise>;
    using payload_t = Promise::payload_type;

public:
    this_coro_promise_awaiter() = delete;

    using promise_type = base_t::promise_type;


    using base_t::await_ready;
    using base_t::await_resume;
    using base_t::base_t;


    template<class T>
    decltype(auto) await_suspend(std::coroutine_handle<T> caller) noexcept {
        this_coro_promise_awaiter_base<payload_t>::inject_payload_into(base_t::handle);
        return base_t::await_suspend(caller);
    }
};
}
