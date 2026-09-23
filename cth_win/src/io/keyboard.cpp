#include "cth/win/io/keyboard.hpp"
#include "cth/coro/global_switch.hpp"

// private
#include "../win_include.hpp"
#include "win_key_convert.hpp"

#include "hidusage.h"

#include <algorithm>
#include <mutex>
#include <print>
#include <ranges>
#include <thread>
#include <vector>

namespace cth::win {
namespace {
    class keybd_hook_manager {
    public:
        keybd_hook_manager() = default;
        ~keybd_hook_manager() { stop_hook(); }

        keybd_hook_manager(keybd_hook_manager const&) = delete;
        keybd_hook_manager& operator=(keybd_hook_manager const&) = delete;

        void subscribe(keybd_event_queue& queue) {
            {
                std::scoped_lock const _{_mutex};
                _queues.push_back(&queue);
            }
            _hookSwitch.acquire();
        }

        void unsubscribe(keybd_event_queue& queue) {
            {
                std::scoped_lock const _{_mutex};
                std::erase(_queues, &queue);
            }
            _hookSwitch.release();
        }

        void push_event(cth::io::key_update update) {
            if(update.data.exKey.key == ::cth::io::Key::NONE)
                return;

            std::scoped_lock const _{_mutex};
            for(auto* queue : _queues)
                queue->push(update);
        }

        static LRESULT CALLBACK hook_callback(int n_code, WPARAM w_param, LPARAM l_param);

    private:
        std::vector<keybd_event_queue*> _queues;
        std::mutex _mutex;
        std::jthread _hookThread;

        os::unique_native_handle _stopEvent{CreateEventW(nullptr, TRUE, FALSE, nullptr)};

        // global_switch wired directly to our internal start/stop methods
        cth::co::global_switch _hookSwitch{
            [this] { start_hook(); },
            [this] { stop_hook(); }
        };

        void start_hook() { _hookThread = std::jthread{[this] { thread_proc(); }}; }

        void stop_hook() {
            SetEvent(_stopEvent.get());

            if(_hookThread.joinable())
                _hookThread.join();

            ResetEvent(_stopEvent.get());
        }

        void thread_proc() {
            // 1. Create a "Message-Only" Window (Invisible, doesn't show in taskbar)
            wnd_ptr hwnd{
                CreateWindowExW(
                    0,
                    L"Static",
                    L"RawInputWindow",
                    0,
                    0,
                    0,
                    0,
                    0,
                    HWND_MESSAGE,
                    nullptr,
                    nullptr,
                    nullptr
                )
            };

            CTH_WIN_STABLE_THROW(!hwnd, "failed to create hwnd") {}

            // 2. Register for Keyboard Raw Input
            RAWINPUTDEVICE rid{
                .usUsagePage = HID_USAGE_PAGE_GENERIC,
                .usUsage = 0x06,
                .dwFlags = RIDEV_INPUTSINK,
                .hwndTarget = static_cast<HWND>(hwnd.get())
            };

            if(!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
                return;

            HANDLE const stopHandle = _stopEvent.get();
            while(MsgWaitForMultipleObjects(1, &stopHandle, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0) {
                MSG msg;
                while(PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                    if(msg.message == WM_INPUT) {
                        UINT dwSize = sizeof(RAWINPUT);
                        RAWINPUT lpb{};

                        GetRawInputData(
                            reinterpret_cast<HRAWINPUT>(msg.lParam),
                            RID_INPUT,
                            &lpb,
                            &dwSize,
                            sizeof(RAWINPUTHEADER)
                        );

                        if(lpb.header.dwType == RIM_TYPEKEYBOARD)
                            push_event(to_key_update(lpb.data.keyboard));
                    }
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
            }
        }
    };

    inline keybd_hook_manager keybdManager{};

    LRESULT CALLBACK keybd_hook_manager::hook_callback(int n_code, WPARAM w_param, LPARAM l_param) {
        if(n_code == HC_ACTION) {
            auto const keyStruct = *reinterpret_cast<KBDLLHOOKSTRUCT const*>(l_param);
            auto const event = to_key_update(keyStruct);

            keybdManager.push_event(event);
        }
        return CallNextHookEx(nullptr, n_code, w_param, l_param);
    }

}
}


namespace cth::win {

keybd_event_queue::keybd_event_queue() { keybdManager.subscribe(*this); }

keybd_event_queue::~keybd_event_queue() { keybdManager.unsubscribe(*this); }

[[nodiscard]] bool keybd_event_queue::empty() const {
    std::scoped_lock const _{_queueMtx};
    return _queue.empty();
}

[[nodiscard]] size_t keybd_event_queue::size() const {
    std::scoped_lock const _{_queueMtx};
    return _queue.size();
}

[[nodiscard]] keybd_event_queue::event_t keybd_event_queue::front() const {
    std::scoped_lock const _{_queueMtx};
    return _queue.front();
}

[[nodiscard]] auto keybd_event_queue::peek() const -> std::optional<event_t> {
    std::scoped_lock const _{_queueMtx};

    if(_queue.empty())
        return std::nullopt;

    return _queue.front();
}

[[nodiscard]] keybd_event_queue::event_t keybd_event_queue::pop() {
    std::scoped_lock const _{_queueMtx};
    auto const result = _queue.front();
    _queue.pop();
    if(_queue.empty())
        _readyFence.reset();
    return result;
}

[[nodiscard]] auto keybd_event_queue::pop_queue() -> std::vector<event_t> {
    std::scoped_lock const _{_queueMtx};
    std::vector<event_t> events;
    while(!_queue.empty()) {
        events.push_back(_queue.front());
        _queue.pop();
    }
    _readyFence.reset();
    return events;
}

void keybd_event_queue::clear() {
    std::scoped_lock const _{_queueMtx};
    while(!_queue.empty())
        _queue.pop();
    _readyFence.reset();
}

void keybd_event_queue::push(event_t event) {
    std::scoped_lock const _{_queueMtx};
    _queue.push(std::move(event));
    _readyFence.signal();
}

} // namespace cth::win

namespace cth::win::keybd {

void send(cth::io::key_state state) {
    auto input = win::to_win_input(state);
    SendInput(1, &input, sizeof(INPUT));
}

void send(std::span<cth::io::key_state const> states) {
    std::vector inputs{
        std::from_range,
        states | std::views::transform([](auto const& state) { return win::to_win_input(state); })
    };

    SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
}

} // namespace cth::win::io
