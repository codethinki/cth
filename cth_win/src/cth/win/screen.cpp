#include "cth/win/screen.hpp"

#include "cth/win/string.hpp"

#include "win_include.hpp"

#include <ShellScalingApi.h>
#include <dwmapi.h>


#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Shcore.lib")

namespace cth::win {

hwnd_t desktop_handle() { return GetDesktopWindow(); }


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
        CTH_WIN_STABLE_THROW(result == 0, "failed to register window class: {}", to_string(wClassName)) {}

        classOpt = {wc.hInstance, wClassName};
    }

    window_t window{
        .classOpt{std::move(classOpt)},
        .handle{CreateWindowExW(
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
        )}
    };

    CTH_WIN_STABLE_THROW(window.handle == nullptr, "failed to create temp window: {}", name) {}

    return window;
}

}


namespace cth::win {
rect_t to_rect(RECT rect) {
    auto const left = static_cast<ssize_t>(rect.left);
    auto const top = static_cast<ssize_t>(rect.top);
    auto const right = static_cast<ssize_t>(rect.right);
    auto const bottom = static_cast<ssize_t>(rect.bottom);


    return {left, top, static_cast<size_t>(right - left), static_cast<size_t>(bottom - top)};
}
}


namespace cth::win {


std::vector<monitor_t> enum_monitors() {
    std::vector<monitor_t> out{};
    out.reserve(4); // pulled that number out my ass

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
namespace {
    using mapping_t = std::pair<DPI_AWARENESS_CONTEXT, DpiAwareness>;
    inline std::array<mapping_t, 4> const mappings = {
        {{DPI_AWARENESS_CONTEXT_UNAWARE, DpiAwareness::NONE},
         {DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED, DpiAwareness::GDI_SCALED},
         {DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE, DpiAwareness::PER_MONITOR_LEGACY},
         {DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2, DpiAwareness::PER_MONITOR}}
    };

    DpiAwareness from_win(DPI_AWARENESS_CONTEXT ctx) {
        for(auto const& m : mappings)
            if(ctx == m.first)
                return m.second;

        CTH_CRITICAL(false, "unknown dpi awareness") {}
        return DpiAwareness::NONE;
    }

    DPI_AWARENESS_CONTEXT to_win(DpiAwareness awareness) {
        for(auto const& m : mappings) {
            if(m.second == awareness)
                return m.first;
        }
        CTH_CRITICAL(false, "unknown dpi awareness") {}
        return DPI_AWARENESS_CONTEXT_UNAWARE;
    }


}


DpiAwareness set_thread_dpi_awareness(DpiAwareness awareness) {
    return from_win(SetThreadDpiAwarenessContext(to_win(awareness)));
}
rect_t window_rect(hwnd_t hwnd) {
    RECT out;
    auto const result = GetWindowRect(static_cast<HWND>(hwnd), &out);
    CTH_WIN_STABLE_THROW(!result, "failed to get window rect of [{}]", hwnd) {}
    return to_rect(out);
}
size_t window_frame_count(hwnd_t hwnd) {

    DWM_TIMING_INFO timingInfo{.cbSize = sizeof(DWM_TIMING_INFO)};

    auto hresult = DwmGetCompositionTimingInfo(static_cast<HWND>(hwnd), &timingInfo);

    CTH_WIN_STABLE_THROW(FAILED(hresult), "failed to get frame count of {}", hwnd) {}

    return timingInfo.cFrame;
}


namespace {
    struct scoped_stretch_blt_mode {
        scoped_stretch_blt_mode(HDC hdc, int mode) : _hdc{hdc}, _prevMode{mode} {
            swapMode();
            CTH_STABLE_THROW(_prevMode == 0, "failed to swap stretch blt mode") {}
        }
        ~scoped_stretch_blt_mode() { swapMode(); }

    private:
        void swapMode() { _prevMode = SetStretchBltMode(_hdc, _prevMode); }

        HDC _hdc;
        int _prevMode;
    };
}


double monitor_scale(hbmp_t bmp, ssize_t x, ssize_t y) {
    scoped_dpi_awareness _{DpiAwareness::PER_MONITOR};

    auto const hdc = static_cast<HDC>(bmp);

    UINT dpiX = 0;
    UINT dpiY = 0;
    POINT pt = {0, 0};

    auto const dcOrigin = GetDCOrgEx(hdc, &pt);
    CTH_WIN_STABLE_THROW(dcOrigin == 0, "failed to get dc origin") {}

    pt.x += static_cast<LONG>(x);
    pt.y += static_cast<LONG>(y);

    auto const hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    CTH_WIN_STABLE_THROW(hMonitor == nullptr, "failed to get monitor from point") {}

    auto const dpiRes = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    CTH_WIN_STABLE_THROW(FAILED(dpiRes), "failed to get monitor dpi") {}

    CTH_STABLE_THROW(dpiX == 0 || dpiY == 0, "failed to get monitor dpi") {}

    return static_cast<double>(dpiX) / 96.0;
}


double window_scale(hwnd_t window) {
    auto const dpi = GetDpiForWindow(static_cast<HWND>(window));

    CTH_WIN_STABLE_THROW(dpi == 0, "failed to get window dpi") {}

    return dpi / 96.0;
}
void raw_blit(
    hbmp_t src,
    ssize_t src_x,
    ssize_t src_y,
    hbmp_t dst,
    ssize_t dst_x,
    ssize_t dst_y,
    size_t width,
    size_t height
) {
    auto const result = BitBlt(
        static_cast<HDC>(dst),
        static_cast<int>(dst_x),
        static_cast<int>(dst_y),
        static_cast<int>(width),
        static_cast<int>(height),
        static_cast<HDC>(src),
        static_cast<int>(src_x),
        static_cast<int>(src_y),
        SRCCOPY
    );

    CTH_WIN_STABLE_THROW(result == 0, "failed to raw blit") {}
}

void blit(
    hbmp_t src,
    ssize_t src_x,
    ssize_t src_y,
    size_t src_w,
    size_t src_h,
    hbmp_t dst,
    ssize_t dst_x,
    ssize_t dst_y,
    size_t dst_w,
    size_t dst_h
) {
    if(src_w == dst_w && src_h == dst_h) {
        raw_blit(src, src_x, src_y, dst, dst_x, dst_y, src_w, src_h);
        return;
    }

    auto const hdcDst = static_cast<HDC>(dst);

    scoped_stretch_blt_mode _{hdcDst, HALFTONE};

    auto const brushRes = SetBrushOrgEx(hdcDst, 0, 0, nullptr);
    CTH_WIN_STABLE_THROW(brushRes == 0, "failed to set brush origin") {}

    auto const result = StretchBlt(
        hdcDst,
        static_cast<int>(dst_x),
        static_cast<int>(dst_y),
        static_cast<int>(dst_w),
        static_cast<int>(dst_h),
        static_cast<HDC>(src),
        static_cast<int>(src_x),
        static_cast<int>(src_y),
        static_cast<int>(src_w),
        static_cast<int>(src_h),
        SRCCOPY
    );


    CTH_WIN_STABLE_THROW(result == 0, "failed to stretch blit") {}
}


namespace {}


void blit_to_screen(
    hbmp_t src,
    ssize_t src_x,
    ssize_t src_y,
    hbmp_t dst,
    ssize_t dst_x,
    ssize_t dst_y,
    size_t width,
    size_t height,
    bool dpi_aware
) {
    if(dpi_aware) {
        raw_blit(src, src_x, src_y, dst, dst_x, dst_y, width, height);
        return;
    }

    auto const scale = monitor_scale(static_cast<HDC>(dst), static_cast<int>(dst_x), static_cast<int>(dst_y));

    auto const scaledW = static_cast<size_t>(static_cast<double>(width) * scale);
    auto const scaledH = static_cast<size_t>(static_cast<double>(height) * scale);
    auto const screenX = static_cast<ssize_t>(static_cast<double>(dst_x) * scale);
    auto const screenY = static_cast<ssize_t>(static_cast<double>(dst_y) * scale);

    blit(src, src_x, src_y, width, height, dst, screenX, screenY, scaledW, scaledH);
}

void blit_from_screen(
    hbmp_t src,
    ssize_t src_x,
    ssize_t src_y,
    hbmp_t dst,
    ssize_t dst_x,
    ssize_t dst_y,
    size_t width,
    size_t height,
    bool dpi_aware
) {
    if(dpi_aware) {
        raw_blit(src, src_x, src_y, dst, dst_x, dst_y, width, height);
        return;
    }

    auto const scale = monitor_scale(static_cast<HDC>(src), static_cast<int>(src_x), static_cast<int>(src_y));

    auto const scaledX = static_cast<ssize_t>(static_cast<double>(src_x) * scale);
    auto const scaledY = static_cast<ssize_t>(static_cast<double>(src_y) * scale);
    auto const scaledW = static_cast<size_t>(static_cast<double>(width) * scale);
    auto const scaledH = static_cast<size_t>(static_cast<double>(height) * scale);

    blit(src, scaledX, scaledY, scaledW, scaledH, dst, dst_x, dst_y, width, height);
}

}


namespace cth::win::screen {
namespace {
    BITMAPINFO create_bmp_header(long width, long height) {
        return {.bmiHeader{
            .biSize = sizeof(BITMAPINFOHEADER),
            .biWidth = width,

            // negative height for top-down bitmap
            .biHeight = -height,


            .biPlanes = 1,
            .biBitCount = section::PIXEL_SIZE * 8,
            .biCompression = BI_RGB,
        }};
    }
}


section::section(rect_t rect, bool dpi_aware) : _rect{rect}, _dpiAware{dpi_aware} {
    _screenDc.reset(GetDC(nullptr));

    CTH_STABLE_THROW(!_screenDc, "failed to get screen dc") {}

    _memoryDc.reset(CreateCompatibleDC(static_cast<HDC>(_screenDc.get())));

    CTH_STABLE_THROW(!_memoryDc, "failed to create memory dc") {}

    auto const header = create_bmp_header(_rect.width, rect.height);

    auto* hBmpRaw =
        CreateDIBSection(static_cast<HDC>(_memoryDc.get()), &header, DIB_RGB_COLORS, &_data, nullptr, 0);

    CTH_STABLE_THROW(!_data || !hBmpRaw, "failed to create DIB section") {}

    _bmp.reset(hBmpRaw);

    _oldBmp = selectBmp(_bmp.get());
}
section::~section() {
    if(_oldBmp && _memoryDc)
        selectBmp(_oldBmp);
}

void section::relocate(ssize_t x, ssize_t y) {
    _rect.x = x;
    _rect.y = y;
}
void section::flush() { GdiFlush(); }


void* section::selectBmp(void* bmp) { return SelectObject(static_cast<HDC>(_memoryDc.get()), bmp); }

}
