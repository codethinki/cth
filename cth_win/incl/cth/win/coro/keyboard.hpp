#pragma once
#include "cth/win/io/keyboard.hpp"

#include <cth/coro/scheduler.hpp>
#include <cth/coro/utility/fwd.hpp>

#include <coroutine>
#include <functional>
#include <mutex>
#include <optional>


namespace cth::win::co {

class keybd_event_queue;

/**
 * suspends until the queue holds an event, resumes with it
 * @attention one awaiter per queue at a time
 */
class keybd_event_awaiter {
public:
    using queue_t = keybd_event_queue;
    using event_t = ::cth::io::key_update;

    keybd_event_awaiter(queue_t& queue, cth::co::scheduler const& scheduler) noexcept :
        _queue{&queue},
        _scheduler{&scheduler} {}

    [[nodiscard]] bool await_ready() const;

    /**
     * @return false if an event arrived before the waiter was registered, resuming without suspending
     */
    [[nodiscard]] bool await_suspend(std::coroutine_handle<> handle);

    /**
     * @return the next event, @ref std::nullopt on a spurious wake
     */
    [[nodiscard]] std::optional<event_t> await_resume();

private:
    queue_t* _queue;
    cth::co::scheduler const* _scheduler;

public:
    keybd_event_awaiter(keybd_event_awaiter const&) = delete;
    keybd_event_awaiter& operator=(keybd_event_awaiter const&) = delete;
    keybd_event_awaiter(keybd_event_awaiter&&) = delete;
    keybd_event_awaiter& operator=(keybd_event_awaiter&&) = delete;
};


/**
 * this_coro tag building a @ref keybd_event_awaiter with the coroutine's own scheduler
 */
struct [[nodiscard]] next_event_tag : cth::co::this_coro::tag_base {
    constexpr explicit next_event_tag(keybd_event_awaiter::queue_t& queue) noexcept : queue{&queue} {}

    [[nodiscard]] auto operator()(cth::co::this_coro::scheduler_payload auto const& payload) const {
        return keybd_event_awaiter{*queue, payload.scheduler()};
    }

    keybd_event_awaiter::queue_t* queue;
};


/**
 * awaitable @ref cth::win::keybd_event_queue
 */
class keybd_event_queue {
public:
    using queue_t = win::keybd_event_queue;
    using event_t = queue_t::event_t;

    keybd_event_queue();
    ~keybd_event_queue();

    /**
     * awaits the next event
     * @return next event tag, resolves to @ref std::nullopt on a spurious wake
     */
    [[nodiscard]] next_event_tag next() { return next_event_tag{*this}; }

private:
    using callback_t = std::move_only_function<void()>;

    friend class keybd_event_awaiter;

    [[nodiscard]] bool subscribe_once(callback_t callback);
    void notify();

    queue_t _queue{};
    queue_t::key_hook_id_t _hookId{};

    callback_t _callback{};
    std::mutex _callbackMtx{};

public:
    /**
     * the wrapped queue, for everything that does not need to wait
     */
    [[nodiscard]] queue_t& queue() noexcept { return _queue; }
    [[nodiscard]] queue_t const& queue() const noexcept { return _queue; }

    keybd_event_queue(keybd_event_queue const&) = delete;
    keybd_event_queue& operator=(keybd_event_queue const&) = delete;
    keybd_event_queue(keybd_event_queue&&) = delete;
    keybd_event_queue& operator=(keybd_event_queue&&) = delete;
};

}
