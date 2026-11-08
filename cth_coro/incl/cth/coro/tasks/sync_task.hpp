#pragma once
#include "cth/coro/tasks/task_base.hpp"
#include "cth/coro/tasks/awaiters/dev/basic_promise_awaiter.hpp"
#include "cth/coro/tasks/promises/sync_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"


#include <utility>

namespace cth::co::dev {
/**
 * promise part of sync tasks: the fence, the result and `final_sync_awaiter`
 */
template<class T>
struct sync_promise_template : sync_promise_base,
    basic_promise<T, std::suspend_always{}, final_sync_awaiter{}> {};

/**
 * task that a thread can block on
 */
template<sync_promise_type Promise, template<class> class Awaiter> requires raw_promise_type<Promise>
class [[nodiscard]] sync_task_template : public task_base<Promise, Awaiter> {
    using base_t = task_base<Promise, Awaiter>;
    using base_t::task_base;

    friend base_t::promise_type;

    using value_type = typename Promise::value_type;

public:
    /**
     * runs the task on the calling thread until its first suspension
     * @pre not started
     */
    decltype(auto) start() { return this->handle().resume(); }

    /**
     * blocks until the task ends
     * @attention blocks forever if nothing starts the task
     */
    void wait() const noexcept { this->handle().promise().wait(); }

    /**
     * blocks until the task ends, then gives its result
     * @throws what the task throws
     * @note moves the result out
     */
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
/**
 * promise of `sync_task`
 */
template<class T>
struct sync_promise : dev::sync_promise_template<T> {
    sync_task<T> get_return_object() noexcept;
};
}

namespace cth::co {

/**
 * task that a thread can block on without a coroutine
 * @details a caller without a context: `start()` it now, `await()` it later
 * @tparam T result type
 */
template<class T>
class sync_task : public dev::sync_task_template<dev::sync_promise<T>, dev::basic_promise_awaiter> {};
using sync_void_task = sync_task<void>;
}


namespace cth::co::dev {
template<class T>
auto sync_promise<T>::get_return_object() noexcept -> sync_task<T> {
    return {std::coroutine_handle<sync_promise<T>>::from_promise(*this)};
}
}
