#include "cth/win/io/keyboard.hpp"
#include "cth/coro/global_switch.hpp"

// private
#include "cth/win/win_include.hpp"
#include "cth/win/io/win_key_convert.hpp"

#include "hidusage.h"

#include <algorithm>
#include <mutex>
#include <print>
#include <ranges>
#include <thread>
#include <vector>

namespace cth::win::keybd {
namespace {
    class hook_manager {
    public:
        hook_manager() = default;
        ~hook_manager() { stop_hook(); }

        hook_manager(hook_manager const&) = delete;
        hook_manager& operator=(hook_manager const&) = delete;

        void subscribe(event_queue& queue) {
            {
                std::scoped_lock lock{_mutex};
                _queues.push_back(&queue);
            }
            _hookSwitch.acquire();
        }

        void unsubscribe(event_queue& queue) {
            {
                std::scoped_lock lock{_mutex};
                std::erase(_queues, &queue);
            }
            _hookSwitch.release();
        }

        void push_event(cth::io::key_update update) {
            if(update.data.exKey.key == ::cth::io::Key::NONE)
                return;

            std::scoped_lock lock{_mutex};
            for(auto* queue : _queues)
                queue->push(update);
        }

        static LRESULT CALLBACK hook_callback(int n_code, WPARAM w_param, LPARAM l_param);

    private:
        std::vector<event_queue*> _queues;
        std::mutex _mutex;
        std::jthread _hookThread;
        DWORD _hookThreadId{0};

        // global_switch wired directly to our internal start/stop methods
        cth::co::global_switch _hookSwitch{
            [this] { start_hook(); },
            [this] { stop_hook(); }
        };

        void start_hook() { _hookThread = std::jthread{[this](std::stop_token const& stop) { thread_proc(stop); }}; }

        void stop_hook() {
            _hookThread.request_stop();
            if(_hookThreadId != 0)
                PostThreadMessageW(_hookThreadId, WM_QUIT, 0, 0);

            if(_hookThread.joinable())
                _hookThread.join();
            _hookThreadId = 0;
        }

        void thread_proc(std::stop_token const& stop) {
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

            _hookThreadId = GetCurrentThreadId();

            MSG msg;
            while(GetMessageW(&msg, nullptr, 0, 0) > 0 && !stop.stop_requested()) {
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

            _hookThreadId = 0;
        }
    };

    inline hook_manager keybdManager{};

    LRESULT CALLBACK hook_manager::hook_callback(int n_code, WPARAM w_param, LPARAM l_param) {
        if(n_code == HC_ACTION) {
            auto const keyStruct = *reinterpret_cast<KBDLLHOOKSTRUCT const*>(l_param);
            auto const event = to_key_update(keyStruct);

            keybdManager.push_event(event);
        }
        return CallNextHookEx(nullptr, n_code, w_param, l_param);
    }

}
}


namespace cth::win::keybd {

event_queue::event_queue() { keybdManager.subscribe(*this); }

event_queue::~event_queue() { keybdManager.unsubscribe(*this); }

[[nodiscard]] bool event_queue::empty() const {
    std::scoped_lock lock{_queueMtx};
    return _queue.empty();
}

[[nodiscard]] event_queue::event_t event_queue::front() const {
    std::scoped_lock lock{_queueMtx};
    return _queue.front();
}

[[nodiscard]] event_queue::event_t event_queue::pop() {
    std::scoped_lock lock{_queueMtx};
    auto const result = _queue.front();
    _queue.pop();
    return result;
}

[[nodiscard]] auto event_queue::pop_queue() -> std::vector<event_t> {
    std::scoped_lock lock{_queueMtx};
    std::vector<event_t> events;
    while(!_queue.empty()) {
        events.push_back(_queue.front());
        _queue.pop();
    }
    return events;
}

void event_queue::clear() {
    std::scoped_lock lock{_queueMtx};
    while(!_queue.empty())
        _queue.pop();
}

void event_queue::push(event_t event) {
    std::scoped_lock lock{_queueMtx};
    _queue.push(std::move(event));
}

} // namespace cth::win::io

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
