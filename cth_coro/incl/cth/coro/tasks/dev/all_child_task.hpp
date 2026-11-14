#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/tasks/promises/basic_promise.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/ptr/ref_wrapper.hpp>

#include <atomic>
#include <coroutine>
#include <cstddef>
#include <utility>


namespace cth::co::dev {
/**
 * shared state of the children of one `co::all()`
 */
struct all_state {
    /**
     * the children that did not end, plus one while the awaiter starts them
     */
    std::atomic<size_t> count{};

    co::context const* callerContext = nullptr; ///< the caller resumes on it
};

/**
 * final awaiter of a child of `co::all()`
 * @details the last child resumes the caller, its `raw_promise_base::continuation`, on its context
 */
struct all_final_awaiter {
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

    /**
     * counts the end of the child, the last child resumes the caller
     * @param h coroutine of the child, the awaiter of `co::all()` destroys it
     * @return see `resume_on()` for the last child, else `std::noop_coroutine()`
     * @attention the post can throw inside noexcept: terminates
     */
    template<class Promise>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> h) noexcept {
        auto& state = h.promise().state();
        // not the last: the last child or the start resumes the caller, the caller can destroy this coroutine at once
        if(state.count.fetch_sub(1, std::memory_order_acq_rel) != 1) return std::noop_coroutine();

        // the last: nothing else touches the state or this coroutine until the caller resumes
        return resume_on(*state.callerContext, h.promise().continuation);
    }

    constexpr void await_resume() const noexcept {}
};

template<class T, payload Pyld>
struct all_child_promise;

/**
 * return type of the coroutine of a child of `co::all()`
 * @details owns nothing: the awaiter of `co::all()` destroys the coroutine
 * @tparam T result type of the child
 * @tparam Pyld payload of the caller
 */
template<class T, payload Pyld>
struct all_child_task {
    using promise_type = all_child_promise<T, Pyld>;

    std::coroutine_handle<promise_type> handle;
};

/**
 * promise of a child of `co::all()`
 * @details
 * - a `this_coro` coroutine with the context and the payload of the caller
 * - starts when the awaiter of `co::all()` resumes it
 * - stores the result or the exception, see `raw_promise`
 * - `raw_promise_base::continuation`: the caller
 * @tparam T result type of the child
 * @tparam Pyld payload of the caller
 */
template<class T, payload Pyld>
struct all_child_promise : basic_promise<T, std::suspend_always{}, all_final_awaiter{}>, this_coro_promise_base<Pyld> {
    /**
     * constructs from the coroutine parameters
     * @param state of the children
     * @param context of the caller, the child starts on it
     * @param payload of the caller
     */
    template<class... Args>
    explicit all_child_promise(all_state& state, co::context const& context, Pyld const& payload, Args&&...) :
        this_coro_promise_base<Pyld>{context, payload}, _state{state} {}

    [[nodiscard]] all_child_task<T, Pyld> get_return_object() noexcept {
        return {std::coroutine_handle<all_child_promise>::from_promise(*this)};
    }

    [[nodiscard]] all_state& state() const noexcept { return *_state; }

private:
    cth::ref_wrapper<all_state> _state;
};

/**
 * awaits the awaitable, the promise stores its result or its exception
 * @tparam T result type of the awaitable
 * @tparam Pyld payload of the caller
 * @tparam Awaitable a reference -> the coroutine keeps the reference, else owns the awaitable
 * @param state the promise constructor reads it
 * @param context the promise constructor reads it
 * @param payload the promise constructor reads it
 * @param awaitable to await
 */
template<class T, payload Pyld, class Awaitable>
auto all_child(
    [[maybe_unused]] all_state& state,
    [[maybe_unused]] co::context const& context,
    [[maybe_unused]] Pyld const& payload,
    Awaitable awaitable
) -> all_child_task<T, Pyld> {
    co_return co_await std::forward<Awaitable>(awaitable); // void: co_await, then co_return
}
}
