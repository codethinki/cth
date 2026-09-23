#pragma once
#include "cth/win/io/keyboard.hpp"

#include <cth/coro/awaiters/base/osfence_awaiter_base.hpp>

namespace cth::win::co {

class keybd_event_queue;

/**
 * suspends until the queue holds an event, resumes with it
 * @attention one awaiter per queue at a time
 */
class keybd_event_awaiter : public cth::co::osfence_awaiter_base<> {
public:
    using queue_t = win::keybd_event_queue;
    using event_t = ::cth::io::key_update;

    explicit keybd_event_awaiter(queue_t& queue) noexcept;

    /** @return the next event */
    [[nodiscard]] event_t await_resume();

private:
    queue_t& _queue;

public:
    keybd_event_awaiter(keybd_event_awaiter const&) = delete;
    keybd_event_awaiter& operator=(keybd_event_awaiter const&) = delete;
    keybd_event_awaiter(keybd_event_awaiter&&) noexcept = default;
    keybd_event_awaiter& operator=(keybd_event_awaiter&&) noexcept = delete;
};


/**
 * awaitable @ref cth::win::keybd_event_queue
 */
class keybd_event_queue : private win::keybd_event_queue {
public:
    using queue_t = win::keybd_event_queue;
    using event_t = queue_t::event_t;

    keybd_event_queue() = default;
    ~keybd_event_queue() = default;

    /** awaits and consumes the next event */
    [[nodiscard]] keybd_event_awaiter next() {
        return keybd_event_awaiter{static_cast<queue_t&>(*this)};
    }

    using queue_t::front;
    using queue_t::peek;
    using queue_t::pop;
    using queue_t::pop_queue;
    using queue_t::push;
    using queue_t::clear;
    using queue_t::size;
    using queue_t::empty;

    keybd_event_queue(keybd_event_queue const&) = delete;
    keybd_event_queue& operator=(keybd_event_queue const&) = delete;
    keybd_event_queue(keybd_event_queue&&) = delete;
    keybd_event_queue& operator=(keybd_event_queue&&) = delete;
};

}
