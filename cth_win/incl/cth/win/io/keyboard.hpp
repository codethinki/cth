#pragma once
#include "cth/io/keybd/keys.hpp"

#include <cth/os/fence.hpp>

#include <mutex>
#include <optional>
#include <queue>
#include <span>
#include <vector>


namespace cth::win::keybd {

void send(::cth::io::key_state);
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
     * @pre !@ref empty()
     */
    [[nodiscard]] event_t front() const;

    /**
     * peeks the queues front
     * @return front, @ref std::nullopt if @ref empty()
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

    /** pushes onto the queue and signals @ref fence() */
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
    [[nodiscard]] size_t size() const;

    /**
     * true if empty
     */
    [[nodiscard]] bool empty() const;
};

} // namespace cth::win
