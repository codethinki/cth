#include "cth/win/coro/keyboard.hpp"

#include <cth/io/log.hpp>

#include <utility>


namespace cth::win::co {

keybd_event_queue::keybd_event_queue() {
    _hookId = _queue.add_key_hook([this] { notify(); });
}

keybd_event_queue::~keybd_event_queue() { _queue.remove_key_hook(_hookId); }

bool keybd_event_queue::subscribe_once(callback_t callback) {
    std::scoped_lock _{_callbackMtx};

    if(!_queue.empty())
        return false;

    if(_callback) {
        CTH_CRITICAL(true, "only one keyboard event awaiter may wait at a time") {}
        return false;
    }

    _callback = std::move(callback);
    return true;
}

void keybd_event_queue::notify() {
    callback_t callback;

    {
        std::scoped_lock _{_callbackMtx};
        callback = std::exchange(_callback, nullptr);
    }

    if(callback)
        callback();
}


bool keybd_event_awaiter::await_ready() const { return !_queue->queue().empty(); }

bool keybd_event_awaiter::await_suspend(std::coroutine_handle<> handle) {
    return _queue->subscribe_once(
        [handle, scheduler = _scheduler] { scheduler->post([handle] { handle.resume(); }); }
    );
}

auto keybd_event_awaiter::await_resume() -> std::optional<event_t> {
    if(_queue->queue().empty())
        return std::nullopt;

    return _queue->queue().pop();
}

}
