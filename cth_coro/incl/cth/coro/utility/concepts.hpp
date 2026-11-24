#pragma once
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <type_traits>


namespace cth::co {

/**
 * `true` if the awaiter of `T` derives `this_coro_awaiter_base`
 * @details the awaiter resumes the caller on its context itself
 */
template<class T>
concept this_coro_awaitable =
    co::awaitable<T> && std::is_base_of_v<this_coro_awaiter_base, std::remove_cvref_t<co::awaiter_t<T>>>;

/**
 * `true` if `T` is awaitable, not a `this_coro_awaitable` and not a tag
 * @details a `this_coro` coroutine captures it, see `dev::steal()`
 * @attention its awaiter can resume the capture on its own thread: that thread then posts to the context of
 *  the caller, end or join it before the context, see `scheduler::~scheduler()`
 */
template<class T>
concept foreign_awaitable = co::awaitable<T> && !this_coro_awaitable<T> && !co::this_coro::tag<T>;

/**
 * `true` if `T` is a scope of `executor::detach()`: it counts the detached tasks
 * @details `enter()` at the start of a task, `leave()` after the destruction of its coroutine
 * @see `simple_counting_scope`, `catching_scope`
 */
template<class T>
concept detach_scope = requires(T& scope) {
    { scope.enter() } noexcept;
    { scope.leave() } noexcept;
};

/**
 * `true` if `T` derives `sync_promise_base`
 */
template<class T>
concept sync_promise_type = std::is_base_of_v<sync_promise_base, T>;

namespace dev {
    /**
     * `true` if `Promise` attaches to a `CallerPromise` without a throw
     */
    template<class Promise, class CallerPromise>
    concept nothrow_attachable = requires(Promise& p, CallerPromise const& c) { { p.attach(c) } noexcept; };
}

}
