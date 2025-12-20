#include "cth/win/screen.hpp"

#include "cth/win/string.hpp"

#include "win_include.hpp"

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

namespace cth::win {

rect_t window_rect(hwnd_t hwnd) {
    RECT out;
    auto const result = GetWindowRect(static_cast<HWND>(hwnd), &out);
    CTH_WIN_STABLE_THROW(!result, "failed to get window rect of [{}]", hwnd) {}
    return to_rect(out);
}

}


namespace cth::win::screen {
namespace {
    BITMAPINFO create_bmp_header(long width, long height) {
        return {
            .bmiHeader{
                .biSize = sizeof(BITMAPINFOHEADER),
                .biWidth = width,

                // Negative height requests a "Top-Down" DIB. 
                // This means data[0] is the Top-Left pixel. (Standard GDI is Bottom-Up)
                .biHeight = -height,


                .biPlanes = 1,
                .biBitCount = section::PIXEL_SIZE * 8,
                // 32-bit (BGRA) for easy alignment
                .biCompression = BI_RGB,
            }
        };
    }
}



section::section(rect_t rect) : _rect{rect} {
    _screenDc.reset(GetDC(nullptr));

    CTH_STABLE_THROW(!_screenDc, "failed to get screen dc") {}

    _memoryDc.reset(CreateCompatibleDC(static_cast<HDC>(_screenDc.get())));

    CTH_STABLE_THROW(!_memoryDc, "failed to create memory dc") {}

    auto const header = create_bmp_header(_rect.width, rect.height);

    auto* hBmpRaw = CreateDIBSection(
        static_cast<HDC>(_memoryDc.get()),
        &header,
        DIB_RGB_COLORS,
        &_data,
        nullptr,
        0
    );

    CTH_STABLE_THROW(!_data || !hBmpRaw, "failed to create DIB section") {}

    _bmp.reset(hBmpRaw);

    _oldBmp = selectBmp(_bmp.get());
}
section::~section() { if(_oldBmp && _memoryDc) selectBmp(_oldBmp); }

void section::relocate(ssize_t x, ssize_t y) {
    _rect.x = x;
    _rect.y = y;
}
void section::flush() { GdiFlush(); }

void section::write_bmp(
    void* src,
    ssize_t src_x,
    ssize_t src_y,
    void* dst,
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

    CTH_WIN_STABLE_THROW(result == 0, "failed to write bitmap") {}
}
void* section::selectBmp(void* bmp) { return SelectObject(static_cast<HDC>(_memoryDc.get()), bmp); }

}
