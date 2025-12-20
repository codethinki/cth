#include "cth/win/win_types.hpp"

#include "win_include.hpp"

namespace cth::win {
void global_lock_deleter::operator()(void* ptr) const { GlobalUnlock(ptr); }

void bmp_deleter::operator()(void* ptr) const { DeleteObject(ptr); }

void hdc_deleter::operator()(void* hdc) const { DeleteDC(static_cast<HDC>(hdc)); }
void hdc_releaser::operator()(void* hdc) const {
    ReleaseDC(nullptr, static_cast<HDC>(hdc));
}

void hwnd_deleter::operator()(hwnd_t handle) const { DestroyWindow(static_cast<HWND>(handle)); }
void handle_deleter::operator()(void* handle) const { CloseHandle(handle); }
window_class_t::~window_class_t() noexcept {
    if(id.empty()) return;
    UnregisterClassW(id.data(), static_cast<HINSTANCE>(hInstance));
}


}

namespace cth::except {

win_exception::win_exception(
    std::string_view msg,
    Severity const severity,
    std::source_location const& loc,
    std::stacktrace trace
) : default_exception{std::format("cth_win: {}", msg), severity, loc, std::move(trace)} {
    add("GetLastError(): {}", GetLastError());
}
}
