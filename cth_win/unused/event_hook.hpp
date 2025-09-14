#pragma once
#include "win_include.hpp"
#include "../io/log.hpp"

#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <shared_mutex>

namespace cth::win::dev {
VOID CALLBACK defaultEventCallback(
    HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG id_object,
    LONG id_child, DWORD dw_event_thread,
    DWORD dw_ms_event_time
);
}

namespace cth::win {


struct window_event_cb_args {
    HWINEVENTHOOK hook;
    DWORD event;
    HWND hwnd;
    LONG idObject;
    LONG idChild;
    DWORD dwEventThread;
    DWORD dwMsEventTime;
};

using win_event_cb_fn = std::function<void(window_event_cb_args const&)>;



class window_event_hook {
public:
    using cb_args = window_event_cb_args;
    using cb_fn = win_event_cb_fn;

    explicit window_event_hook(WINEVENTPROC callback) : _callback{callback} {}

    void add(HWND hwnd, cb_fn const& handler) {
        if(empty()) hook();

        std::unique_lock _{_mountsMtx};
        _mounts[hwnd].insert(handler);
    }
    void remove(HWND hwnd, cb_fn const& handler) {
        CTH_CRITICAL(!contains(hwnd, handler), "no mounts for hwnd found, hwnd:[{}]", reinterpret_cast<size_t>(hwnd)) {}

        std::unique_lock _{_mountsMtx};

        auto& mounts = _mounts[hwnd];
        mounts.erase(handler);
        if(mounts.empty()) _mounts.erase(hwnd);
        if(_mounts.empty()) unhook();
    }

private:
    void process(cb_args const& args)  {
        std::shared_lock lock{_mountsMtx};

        if(!_mounts.contains(args.hwnd)) return;

        auto const contained = _mounts.at(args.hwnd);
        lock.unlock();

        for(auto const& func : contained)
            func(args);
    }

    struct unhook_deleter {
        void operator()(HWINEVENTHOOK hook) const { UnhookWinEvent(hook); }
    };

    using hook_t = std::unique_ptr<std::remove_pointer_t<HWINEVENTHOOK>, unhook_deleter>;

    void hook() {
        auto const hook = SetWinEventHook(
            EVENT_OBJECT_CREATE, EVENT_OBJECT_DESTROY, nullptr, _callback, 0, 0,
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS
        );


        CTH_STABLE_ERR(hook == nullptr, "failed to hook the remount_map") {}

        _hook = hook_t{hook};
    }
    void unhook() { _hook = nullptr; }



    [[nodiscard]] bool empty() {
        std::shared_lock _{_mountsMtx};
        return _mounts.empty();
    }
    [[nodiscard]] bool contains(HWND hwnd) {
        std::shared_lock _{_mountsMtx};
        return _mounts.contains(hwnd);
    }
    [[nodiscard]] bool contains(HWND hwnd, cb_fn const& handler) {
        std::shared_lock _{_mountsMtx};
        if(!_mounts.contains(hwnd)) return false;
        return _mounts[hwnd].contains(handler);
    }

    WINEVENTPROC _callback;

    hook_t _hook = nullptr;

    std::mutex _mountsMtx{};
    std::map<HWND, std::set<cb_fn>> _mounts{};

    friend decltype(dev::defaultEventCallback);

public:
    static window_event_hook global;
};

inline window_event_hook window_event_hook::global{dev::defaultEventCallback};

}


namespace cth::win::dev {
inline VOID CALLBACK defaultEventCallback(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG id_object, LONG id_child, DWORD dw_event_thread,
    DWORD dw_ms_event_time) {
    if(id_object != OBJID_WINDOW) return;

    auto& eventHook = window_event_hook::global;

    eventHook.process({hook, event, hwnd, id_object, id_child, dw_event_thread, dw_ms_event_time});
}
}
