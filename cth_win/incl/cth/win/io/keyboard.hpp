#pragma once
#include "cth/io/keybd/keys.hpp"

#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <span>
#include <vector>


namespace cth::win::keybd {

void send(::cth::io::key_state);
void send(std::span<::cth::io::key_state const> states);


struct event_queue {
    using event_t = ::cth::io::key_update;

    /**
     * notification hook, invoked once per @ref push()
     * @attention key hook must not change queue's hook registration
     */
    using key_hook_t = std::move_only_function<void()>;
    /**
     * index of the hook's slot
     * @details @ref remove_key_hook() empties the slot and @ref add_key_hook() reuses it, so ids are
     *  handed out again -- an id kept past its own removal addresses whatever hook took the slot over
     */
    using key_hook_id_t = std::size_t;

    event_queue();
    event_queue(event_queue const&) = delete;
    event_queue(event_queue&&) = delete;
    ~event_queue();

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

    /**
     * pushes onto the queue, notifies every hook
     * @param event to push
     */
    void push(event_t event);

    /**
     * registers a notification hook
     * @param hook see @ref key_hook_t
     * @pre hook != nullptr
     * @return id to pass to @ref remove_key_hook()
     */
    key_hook_id_t add_key_hook(key_hook_t hook);

    /**
     * unregisters a hook
     * @param id from @ref add_key_hook()
     * @pre @ref key_hook_active(id)
     */
    void remove_key_hook(key_hook_id_t id);

    /**
     * checks whether an id addresses a registered hook
     * @param id from @ref add_key_hook()
     * @return false for a removed id and for an id this queue never handed out
     */
    [[nodiscard]] bool key_hook_active(key_hook_id_t id) const;

private:
    /**
     * invokes every registered hook, called by @ref push() with the queue mutex released
     */
    void notify_hooks();

    std::queue<event_t> _queue{};
    mutable std::mutex _queueMtx{};

    // separate lock: hooks are user code and must not run under _queueMtx
    // slot index is the id, a removed slot stays in place as empty and gets reused
    std::vector<key_hook_t> _hooks{};
    mutable std::mutex _hooksMtx{};

public:
    /**
     * size of current buffer queue
     */
    [[nodiscard]] size_t size() const;

    /**
     * true if empty
     */
    [[nodiscard]] bool empty() const;
};

} // namespace cth::win::io
