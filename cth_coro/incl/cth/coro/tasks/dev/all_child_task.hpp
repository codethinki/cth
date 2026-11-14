#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/data/optional.hpp>
#include <cth/meta/concepts.hpp>
#include <cth/ptr/ref_wrapper.hpp>

#include <atomic>
#include <coroutine>
#include <cstddef>
#include <exception>
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

    std::coroutine_handle<> caller = nullptr; ///< resumes after the last child

    co::context const* callerContext = nullptr; ///< the caller resumes on it
};

/**
 * result or exception of one child of `co::all()`
 * @tparam T result type of the child
 */
template<class T>
struct all_slot {
    dt::optional<T> result{}; ///< `std::nullopt` -> an exception or no end yet

    std::exception_ptr exception = nullptr; ///< `nullptr` -> no exception
};

/**
 * final awaiter of a child of `co::all()`
 * @details the last child resumes the caller on its context
 */
struct all_final_awaiter {
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

    /**
     * counts the end of the child, the last child resumes the caller
     * @param h coroutine of the child, the awaiter of `co::all()` destroys it
     * @return the caller if this child is the last and `context::current()` is the context of the caller, else
     *  `std::noop_coroutine()`
     * @attention the post can throw inside noexcept: terminates
     */
    template<class Promise>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> h) noexcept {
        auto& state = h.promise().state();
        // not the last: the last child or the start resumes the caller, the caller can destroy this coroutine at once
        if(state.count.fetch_sub(1, std::memory_order_acq_rel) != 1) return std::noop_coroutine();

        // the last: nothing else touches the state until the caller resumes
        auto const caller = state.caller;
        auto const* target = state.callerContext;
        if(context::current() == target) return caller;
        target->post([caller] { caller.resume(); });
        return std::noop_coroutine();
    }

    constexpr void await_resume() const noexcept {}
};

template<payload Pyld>
struct all_child_promise;

/**
 * return type of the coroutine of a child of `co::all()`
 * @details owns nothing: the awaiter of `co::all()` destroys the coroutine
 * @tparam Pyld payload of the caller
 */
template<payload Pyld>
struct all_child_task {
    using promise_type = all_child_promise<Pyld>;

    std::coroutine_handle<promise_type> handle;
};

/**
 * promise of a child of `co::all()`
 * @details
 * - a `this_coro` coroutine with the context and the payload of the caller
 * - starts when the awaiter of `co::all()` resumes it
 * @tparam Pyld payload of the caller
 */
template<payload Pyld>
struct all_child_promise : this_coro_promise_base<Pyld> {
    /**
     * constructs from the coroutine parameters
     * @param state of the children
     * @param context of the caller, the child starts on it
     * @param payload of the caller
     */
    template<class... Args>
    explicit all_child_promise(all_state& state, co::context const& context, Pyld const& payload, Args&&...) :
        this_coro_promise_base<Pyld>{context, payload}, _state{state} {}

    [[nodiscard]] all_child_task<Pyld> get_return_object() noexcept {
        return {std::coroutine_handle<all_child_promise>::from_promise(*this)};
    }

    [[nodiscard]] constexpr std::suspend_always initial_suspend() noexcept { return {}; }

    [[nodiscard]] constexpr all_final_awaiter final_suspend() noexcept { return {}; }

    constexpr void return_void() noexcept {}

    /**
     * the body stores every exception in the slot
     */
    [[noreturn]] void unhandled_exception() noexcept { std::terminate(); }

    [[nodiscard]] all_state& state() const noexcept { return *_state; }

private:
    cth::ref_wrapper<all_state> _state;
};

/**
 * awaits the awaiter, stores its result or its exception in the slot
 * @tparam T result type of the awaiter
 * @tparam Pyld payload of the caller
 * @tparam Awaiter the awaiter, a reference -> the coroutine keeps the reference
 * @param state the promise constructor reads it
 * @param context the promise constructor reads it
 * @param payload the promise constructor reads it
 * @param awaiter to await
 * @param[out] slot gets the result or the exception
 */
template<class T, payload Pyld, class Awaiter>
auto all_child(
    [[maybe_unused]] all_state& state,
    [[maybe_unused]] co::context const& context,
    [[maybe_unused]] Pyld const& payload,
    Awaiter awaiter,
    all_slot<T>& slot
) -> all_child_task<Pyld> {
    try {
        if constexpr(mta::is_void<T>) {
            co_await std::forward<Awaiter>(awaiter);
            slot.result.emplace();
        } else
            slot.result.emplace(co_await std::forward<Awaiter>(awaiter));
    }
    catch(...) {
        slot.exception = std::current_exception();
    }
}
}
