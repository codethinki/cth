#include "cth/win/screen.hpp"

#include "cth/win/string.hpp"
#include "cth/win/win_include.hpp"

namespace cth::win::screen {

namespace {
    rect_t to_rect(RECT rect) {
        auto const left = static_cast<uint32_t>(rect.left);
        auto const top = static_cast<uint32_t>(rect.top);
        auto const right = static_cast<uint32_t>(rect.right);
        auto const bottom = static_cast<uint32_t>(rect.bottom);

        return {left, top, right - left, bottom - top};
    }
}

window_t create_window(std::string_view name, rect_t rect, bool visible, std::string_view class_name) {
    CTH_CRITICAL(name.empty(), "name must not be empty") {}

    auto const implicitClass = class_name.empty();

    auto const wName = to_wstring(name);
    auto const wClassName = implicitClass ? wName : to_wstring(class_name);

    std::optional<window_class_t> classOpt{};

    WNDCLASSEXW existingClass;
    bool registered = GetClassInfoExW(GetModuleHandle(nullptr), wClassName.c_str(), &existingClass);

    if(!registered) {
        WNDCLASSEXW const wc{
            .cbSize = sizeof(wc),
            .lpfnWndProc = DefWindowProc,
            .hInstance = GetModuleHandle(nullptr),
            .lpszClassName = wClassName.c_str(),
        };
        auto const result = RegisterClassExW(&wc);
        CTH_STABLE_WIN_THROW(result == 0, "failed to register window class: {}", to_string(wClassName)) {}

        classOpt = {wc.hInstance, wClassName};
    }

    window_t window{
        .classOpt{std::move(classOpt)},
        .handle{
            CreateWindowExW(
                0,
                wClassName.c_str(),
                wName.c_str(),
                visible ? WS_VISIBLE : 0,
                static_cast<int>(rect.x),
                static_cast<int>(rect.y),
                static_cast<int>(rect.width),
                static_cast<int>(rect.height),
                nullptr,
                nullptr,
                GetModuleHandle(nullptr),
                nullptr
            )
        }
    };

    CTH_STABLE_WIN_THROW(window.handle == nullptr, "failed to create temp window: {}", name) {}

    return window;
}
}

namespace cth::win::screen {

std::vector<monitor_t> enum_monitors() {
    std::vector<monitor_t> out{};
    out.reserve(4); //pulled that number out my ass

    auto enum_lambda = [](HMONITOR handle, HDC, LPRECT m_rect, LPARAM pout) -> BOOL {
        auto& list = *reinterpret_cast<std::vector<monitor_t>*>(pout);
        list.emplace_back(handle, to_rect(*m_rect));
        return TRUE;
    };

    EnumDisplayMonitors(nullptr, nullptr, enum_lambda, reinterpret_cast<LPARAM>(&out));

    return out;
}

}

namespace cth::win::screen {

rect_t desktop_rect() {
    RECT desktop;
    auto const hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);

    return to_rect(desktop);
}
}
