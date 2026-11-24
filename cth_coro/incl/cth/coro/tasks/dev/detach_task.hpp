#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/catching_scope.hpp"
#include "cth/coro/simple_counting_scope.hpp"
#include "cth/coro/tasks/promises/suspend_promise_base.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/ptr/ref_wrapper.hpp>

#include <concepts>
#include <coroutine>
#include <exception>


namespace cth::co::dev {
//TEMP gcc 15.2+: the ramp of a coroutine that starts eagerly races with the thread that resumes or destroys it, the
// task starts lazily: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=127320
#if defined(__GNUC__) && !defined(__clang__)
#define CTH_CORO_LAZY_DETACH
using initial_detach_suspend_t = std::suspend_always;
#else
using initial_detach_suspend_t = std::suspend_never;
#endif

/**
 * scope tag of `executor::detach()` without a scope
 */
struct no_scope {};

/**
 * last base of `detach_promise` in a scope, holds nothing
 * @details the scope owns the coroutine: its owner waits for it before the scheduler ends
 * @details `~scheduler()` does not destroy the coroutine
 */
template<class Scope>
struct detach_owner {
    explicit constexpr detach_owner(co::context const&, std::coroutine_handle<>) noexcept {}
};

/**
 * last base of `detach_promise` without a scope: the scheduler of the detach owns the coroutine
 * @details `~scheduler()` destroys the coroutine if it did not end
 */
template<>
struct detach_owner<no_scope> {
    /**
     * gives the ownership of the coroutine to the scheduler of the context
     * @details calls `context::detach()`
     * @param context the task runs on
     * @param h coroutine of the detached task
     */
    explicit detach_owner(co::context const& context, std::coroutine_handle<> h) :
        _owner{context.detach(h)} {}

    /**
     * gives the owner of the coroutine: the scheduler of the detach
     */
    [[nodiscard]] co::context const& owner() const noexcept { return *_owner; }

private:
    cth::ref_wrapper<co::context const> _owner;
};

/**
 * final awaiter of `detach_task`
 */
struct final_detach_awaiter {
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

    /**
     * destroys the coroutine
     * @details no scope -> releases the coroutine from its owner first, see `detach_owner`
     * @param h coroutine of the detached task
     */
    template<class Promise>
    void await_suspend(std::coroutine_handle<Promise> h) noexcept {
        // before the destroy: the awaiter lives in the coroutine frame
        if constexpr(std::derived_from<Promise, detach_owner<no_scope>>)
            h.promise().owner().release(h);
        // the last action: the destruction leaves the scope at its end, see detach_scope_guard
        h.destroy();
    }

    constexpr void await_resume() const noexcept {}
};

/**
 * first base of `detach_promise`, the task is in the scope for its lifetime
 * @details enters the scope on construction, leaves it on destruction: after the other parts of the promise
 * @tparam Scope `co::simple_counting_scope`, `co::catching_scope`, `no_scope` -> nothing to enter or leave
 */
template<class Scope>
struct detach_scope_guard {
    /**
     * @param scope the task enters
     */
    explicit detach_scope_guard(Scope& scope) noexcept : _scope{scope} { scope.enter(); }

    // the last access to the scope: the deallocation of the frame after it does not touch the scope
    ~detach_scope_guard() { _scope->leave(); }

protected:
    /**
     * the scope the task is in
     */
    [[nodiscard]] Scope& scope() const noexcept { return *_scope; }

private:
    cth::ref_wrapper<Scope> _scope;

public:
    detach_scope_guard(detach_scope_guard const& other) = delete;
    detach_scope_guard(detach_scope_guard&& other) = delete;
    detach_scope_guard& operator=(detach_scope_guard const& other) = delete;
    detach_scope_guard& operator=(detach_scope_guard&& other) = delete;
};

/**
 * guard of a task without a scope, holds nothing
 */
template<>
struct detach_scope_guard<no_scope> {
    explicit constexpr detach_scope_guard(no_scope) noexcept {}
};

template<payload Pyld, class Scope>
struct detach_promise;

/**
 * return type of the coroutine of `executor::detach()`
 * @details owns nothing: the scope owns the coroutine, without a scope the scheduler of the detach
 * @tparam Pyld payload of the detached task
 * @tparam Scope the task is in, see `detach_scope_guard`
 */
template<payload Pyld, class Scope>
struct detach_task {
    using promise_type = detach_promise<Pyld, Scope>;

#ifdef CTH_CORO_LAZY_DETACH
    std::coroutine_handle<promise_type> handle; ///< the coroutine to start
#endif

    /**
     * runs the coroutine until its first suspension
     * @details `CTH_CORO_LAZY_DETACH` -> resumes it, else it runs already
     */
    void start() const {
#ifdef CTH_CORO_LAZY_DETACH
        handle.resume();
#endif
    }
};

/**
 * promise of `detach_task`
 * @details a `this_coro` coroutine: the awaited task attaches to it
 * @tparam Pyld payload of the detached task
 * @tparam Scope the task is in, see `detach_scope_guard`
 */
template<payload Pyld, class Scope>
struct detach_promise :
    detach_scope_guard<Scope>,
    suspend_promise_base<initial_detach_suspend_t{}, final_detach_awaiter{}>,
    this_coro_promise_base<Pyld>,
    detach_owner<Scope> {
    /**
     * constructs from the coroutine parameters, enters the scope
     * @details no scope -> calls `context::detach()` with the coroutine, see `detach_owner`
     * @param scope the task enters
     * @param payload of the detached task
     * @param context the task runs on
     */
    template<class... Args>
    explicit detach_promise(Scope& scope, Pyld const& payload, co::context const& context, Args&&...) :
        detach_scope_guard<Scope>{scope},
        this_coro_promise_base<Pyld>{context, payload},
        detach_owner<Scope>{context, std::coroutine_handle<detach_promise>::from_promise(*this)} {}

    [[nodiscard]] detach_task<Pyld, Scope> get_return_object() noexcept {
#ifdef CTH_CORO_LAZY_DETACH
        return {std::coroutine_handle<detach_promise>::from_promise(*this)};
#else
        return {};
#endif
    }

    constexpr void return_void() noexcept {}

    /**
     * a detached task has no caller for the exception: terminates
     */
    [[noreturn]] void unhandled_exception() noexcept requires (!std::same_as<Scope, co::catching_scope>) {
        std::terminate();
    }

    /**
     * stores the exception in the scope, the scope keeps the first one
     * @details before the destruction of the coroutine: the error is visible after `catching_scope::wait()`
     */
    void unhandled_exception() noexcept requires std::same_as<Scope, co::catching_scope> {
        this->scope().store(std::current_exception());
    }
};
}
