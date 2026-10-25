#pragma once
#include "cth/coro/utility.hpp"
#include "cth/coro/awaiters/raw_native_handle_awaiter.hpp"
#include "cth/coro/func/dev/steal.hpp"
#include "cth/coro/tasks/dev/capture_task.hpp"
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/this_coro/this_coro_base.hpp"
#include "cth/coro/utility/concepts.hpp"

#include <cth/data/optional.hpp>
#include <cth/meta/coro.hpp>

#include <utility>


namespace cth::co {
class executor;

template<class T>
concept native_handle_waitable = requires(T const& value) {
    { value.native_handle() } -> std::same_as<os::native_handle_t>;
};


/**
 * base of this_coro promises, holds the scheduler the coroutine runs on and the payload
 * @details
 * - promises supporting the libs this_coro namespace must publicly inherit from this
 * - the awaiter of a task calls @ref inherit() before the task runs, a spawned task gets both on construction
 */
template<payload Pyld>
struct this_coro_promise_base : this_coro_base<Pyld> {
    /**
     * @param scheduler the coroutine runs on
     * @param payload of the coroutine
     */
    this_coro_promise_base(co::scheduler const& scheduler, Pyld const& payload) :
        _scheduler{&scheduler},
        _payload{payload} {}
    this_coro_promise_base() noexcept = default;

    /**
     * calls the tag with the scheduler and the payload
     * @return awaitable of the tag
     */
    template<this_coro::tag Tag>
    [[nodiscard]] constexpr decltype(auto) await_transform(Tag&& t) {
        static_assert(
            requires(Tag tag, co::scheduler const& s, Pyld& p) { { std::forward<Tag>(tag)(s, p) } -> awaitable; },
            "the tag must be callable with the scheduler and the payload"
        );

        return std::forward<Tag>(t)(*_scheduler, *_payload);
    }

    /**
     * awaits the awaitable as it is, its awaiter resumes this coroutine on its scheduler
     */
    template<this_coro_awaitable Awaitable>
    [[nodiscard]] constexpr auto await_transform(Awaitable&& awaitable) -> awaiter_t<Awaitable> {
        return co::extract_awaiter(std::forward<Awaitable>(awaitable));
    }

    /**
     * waits on the native handle of the waitable, resumes this coroutine on its scheduler
     */
    template<native_handle_waitable Waitable>
    [[nodiscard]] auto await_transform(Waitable const& waitable) {
        return raw_native_handle_awaiter{*_scheduler, waitable.native_handle()};
    }

    /**
     * captures the foreign awaitable, the capture resumes this coroutine on its scheduler
     */
    template<foreign_awaitable Awaitable> requires (!native_handle_waitable<Awaitable>)
    [[nodiscard]] auto await_transform(Awaitable awaitable) -> capture_task<awaited_t<Awaitable>> {
        return dev::steal(std::move(awaitable));
    }

    /**
     * copies the scheduler and the payload of the caller
     * @param caller promise of the awaiting coroutine
     */
    template<class CallerPromise>
    constexpr void inherit(CallerPromise const& caller) {
        static_assert(
            this_coro::promise_injectable_with<this_coro_promise_base, payload_t<CallerPromise>>,
            "the payload of the caller must convert to the payload of the task"
        );

        _scheduler = &caller.scheduler();
        _payload.emplace(caller.payload());
    }

    /**
     * sets the scheduler the coroutine runs on, keeps the payload
     * @param scheduler the coroutine moves to
     */
    constexpr void rebind_scheduler(co::scheduler const& scheduler) noexcept { _scheduler = &scheduler; }

private:
    co::scheduler const* _scheduler = nullptr;
    cth::dt::optional<Pyld> _payload{};

public:
    [[nodiscard]] constexpr co::scheduler const& scheduler() const noexcept { return *_scheduler; }
    [[nodiscard]] constexpr Pyld const& payload() const noexcept { return *_payload; }
};

}
