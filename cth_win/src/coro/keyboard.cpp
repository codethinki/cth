#include "cth/win/coro/keyboard.hpp"

#include <cth/io/log.hpp>

namespace cth::win::co {

keybd_event_awaiter::keybd_event_awaiter(queue_t& queue) noexcept :
    osfence_awaiter_base{queue.fence()}, _queue{queue} {}

auto keybd_event_awaiter::await_resume() -> event_t {
    CTH_CRITICAL(_queue.empty(), "keyboard queue resumed without an event") {}
    return _queue.pop();
}

}
