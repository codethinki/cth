#pragma once
#include "cth/io/keybd/keys.hpp"

#include <mutex>
#include <queue>
#include <span>
#include <vector>


namespace cth::win::keybd {

void send(::cth::io::key_state);
void send(std::span<::cth::io::key_state const> states);


struct event_queue {
    using event_t = ::cth::io::key_update;

    event_queue();
    event_queue(event_queue const&) = delete;
    event_queue(event_queue&&) = delete;
    ~event_queue();

    /**
     * peeks the queues front
     */
    [[nodiscard]] event_t front() const;
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

    /**
     * pushes onto the queue
     * @param event to push
     */
    void push(event_t event);

private:
    std::queue<event_t> _queue{};
    mutable std::mutex _queueMtx{};

public:
    /**
     * size of current buffer queue
     */
    [[nodiscard]] size_t size() const { return _queue.size(); }
    
    /**
     * true if empty
     */
    [[nodiscard]] bool empty() const;
};

} // namespace cth::win::io
