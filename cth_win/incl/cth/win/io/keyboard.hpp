#pragma once
#include "cth/io/keybd/keys.hpp"

#include <cth/os/fence.hpp>
#include <cth/types.hpp>

#include <mutex>
#include <optional>
#include <queue>
#include <span>
#include <vector>


namespace cth::win::keybd {

/**
 * sends the key state as keyboard input
 * @details
 * - sends the scan code: the position, the layout of the receiver makes the character
 * - `PAUSE` -> virtual key, `SendInput()` has no `0xE1` flag
 * @pre the key is no mouse key
 */
void send(::cth::io::key_state);
/**
 * sends the key states as keyboard input
 * @details
 * - sends the scan codes: the positions, the layout of the receiver makes the characters
 * - `PAUSE` -> virtual key, `SendInput()` has no `0xE1` flag
 * @pre no key of `states` is a mouse key
 */
void send(std::span<::cth::io::key_state const> states);

}


namespace cth::win {

struct keybd_event_queue {
    using event_t = ::cth::io::key_update;

    keybd_event_queue();
    keybd_event_queue(keybd_event_queue const&) = delete;
    keybd_event_queue(keybd_event_queue&&) = delete;
    ~keybd_event_queue();

    /**
     * gets the queues front
     * @pre `!empty()`
     */
    [[nodiscard]] event_t front() const;

    /**
     * peeks the queues front
     * @return front, `std::nullopt` if `empty()`
     */
    [[nodiscard]] std::optional<event_t> peek() const;
    /**
     * pops the queues front
     */
    [[nodiscard]] event_t pop();
    /**
     * pops the entire queue
     */
    [[nodiscard]] std::vector<event_t> pop_queue();

    /**
     * flushes the queue
     */
    void clear();

    /** pushes onto the queue and signals `fence()` */
    void push(event_t event);

private:
    std::queue<event_t> _queue{};
    mutable std::mutex _queueMtx{};
    os::fence _readyFence{};

public:
    /**
     * queue readiness fence
     * @return a manual-reset fence which remains signaled while the queue is nonempty
     */
    [[nodiscard]] os::fence const& fence() const noexcept { return _readyFence; }

    /**
     * size of current buffer queue
     */
    [[nodiscard]] uz size() const;

    /**
     * `true` if empty
     */
    [[nodiscard]] bool empty() const;
};

} // namespace cth::win
