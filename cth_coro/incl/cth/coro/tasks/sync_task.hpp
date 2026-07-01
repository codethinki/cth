#pragma once
#include "cth/coro/tasks/task_base.hpp"
#include "cth/coro/tasks/awaiters/this_coro_promise_awaiter.hpp"
#include "cth/coro/tasks/awaiters/dev/basic_promise_awaiter.hpp"
#include "cth/coro/tasks/promises/sync_promise_base.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"


#include <utility>

namespace cth::co::dev {
template<class T>
struct sync_promise_template : sync_promise_base,
    basic_promise<T, std::suspend_always{}, final_sync_awaiter{}> {};

template<sync_promise_type Promise, template<class> class Awaiter> requires raw_promise_type<Promise>
class [[nodiscard]] sync_task_template : public task_base<Promise, Awaiter> {
    using base_t = task_base<Promise, Awaiter>;
    using base_t::task_base;

    friend base_t::promise_type;

    using value_type = typename Promise::value_type;

public:
    decltype(auto) start() { return this->handle().resume(); }

    void wait() const noexcept { this->handle().promise().wait(); }

    [[nodiscard]] value_type await() {
        this->wait();

        auto& exceptionPtr = this->handle().promise().exception;
        if(exceptionPtr)
            std::rethrow_exception(exceptionPtr);

        if constexpr(mta::is_void<value_type>)
            return;
        else
            return std::forward<value_type>(*(this->handle().promise().result));
    }
};
}


namespace cth::co::dev {
template<class T>
struct sync_promise : dev::sync_promise_template<T> {
    sync_task<T> get_return_object() noexcept;
};


template<class T, payload Pyld = this_coro::default_payload>
struct sync_executor_promise : this_coro_promise_base<Pyld>, sync_promise_template<T> {
    sync_executor_task<T, Pyld> get_return_object() noexcept;
};
}

namespace cth::co {

/**
 * Task that can be block waited on without coroutine mechanisms
 * @tparam T return type to await
 */
template<class T>
class sync_task : public dev::sync_task_template<dev::sync_promise<T>, dev::basic_promise_awaiter> {};
using sync_void_task = sync_task<void>;

template<class T, payload Pyld>
class sync_executor_task : public dev::sync_task_template<
        dev::sync_executor_promise<T, Pyld>,
        this_coro_promise_awaiter
    > {};

using sync_executor_void_task = sync_executor_task<void>;
}


namespace cth::co::dev {
template<class T>
auto sync_promise<T>::get_return_object() noexcept -> sync_task<T> {
    return {std::coroutine_handle<sync_promise<T>>::from_promise(*this)};
}

template<class T, payload Pyld>
auto sync_executor_promise<T, Pyld>::get_return_object() noexcept -> sync_executor_task<T, Pyld> {
    return {std::coroutine_handle<sync_executor_promise<T, Pyld>>::from_promise(*this)};
}
}
