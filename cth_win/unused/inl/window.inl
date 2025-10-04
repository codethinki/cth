#pragma once
#include "../win_types.hpp"

namespace cth::win {

inline glm::ivec2 extent(HWND hwnd) {
    auto posExtent = pos_extent(hwnd);
    return {posExtent[2], posExtent[3]};
}

inline glm::ivec2 pos(HWND hwnd) {
    auto posExtent = pos_extent(hwnd);
    return {posExtent.x, posExtent.y};
}

inline glm::ivec4 pos_extent(HWND hwnd) {
    RECT rect;
    auto const result = GetWindowRect(hwnd, &rect);
    CTH_STABLE_ERR(!result, "failed to get window rect") {
        details->add("error code: ", GetLastError());
        throw details->exception();
    }

    auto x = rect.left;
    auto y = rect.top;
    auto width = rect.right - x;
    auto height = rect.bottom - y;

    return {x, y, width, height};
}


inline std::vector<uint8_t> screenshot(HWND hwnd, glm::ivec2 size, glm::ivec2 offset) {
    std::vector<uint8_t> buffer(static_cast<size_t>(size.x * size.y));

    cth::win::screenshot_to(std::span{buffer.data(), buffer.size()}, hwnd, size, offset);
    return buffer;
}
inline void screenshot_to(std::span<uint8_t> buffer, HWND hwnd, glm::ivec2 size, glm::ivec2 offset) {
    using bmp_swap_ptr = std::unique_ptr<std::remove_pointer_t<HGDIOBJ>, std::function<void(HGDIOBJ)>>;

    static cxpr int PIXEL_BITS = 24;
    static cxpr int PIXEL_BYTES = PIXEL_BITS / 8;

    CTH_CRITICAL(buffer.size() < static_cast<size_t>(PIXEL_BYTES * size.x * size.y), "buffer too small") {}


    auto const windowDc = GetDC(hwnd);
    hdc_ptr const copyDc{CreateCompatibleDC(windowDc)};
    CTH_STABLE_ERR(!windowDc, "failed to create copy DC") throw details->exception();


    BITMAPINFO const bmpInfo{
        .bmiHeader{
            .biSize = sizeof(BITMAPINFOHEADER),
            .biWidth = static_cast<LONG>(size.x),
            .biHeight = static_cast<LONG>(size.y),
            .biPlanes = 1,
            .biBitCount = static_cast<WORD>(PIXEL_BITS),
            .biCompression = BI_RGB,
        }
    };
    void* bits = nullptr;

    bmp_ptr const bmp{CreateDIBSection(copyDc.get(), &bmpInfo, DIB_RGB_COLORS, &bits, nullptr, 0)};
    CTH_STABLE_ERR(!bmp, "failed to create DIB section") throw details->exception();

    [[maybe_unused]] bmp_swap_ptr oldBmp{SelectObject(copyDc.get(), bmp.get()), [&copyDc](auto const& old) { SelectObject(copyDc.get(), old); }};

    auto const blitResult = !BitBlt(copyDc.get(), 0, 0, size.x, size.y, windowDc, offset.x, offset.y, SRCCOPY | CAPTUREBLT);
    CTH_STABLE_ERR(!blitResult, "failed to blit to copy") {
        details->add("error code:", GetLastError());
        throw details->exception();
    }

    auto const stride = static_cast<ptrdiff_t>(cth::num::align_to<DWORD>(size.x));

    auto const rowBytes = size.x * PIXEL_BYTES;
    for(size_t i = 0; i < size.y; ++i)
        std::memcpy(&buffer[i * rowBytes], static_cast<uint8_t*>(bits) + i * stride, rowBytes);
}
}

namespace cth::win {

inline void reg_global_raw_input(HWND hwnd) {
    using rid_t = RAWINPUTDEVICE;

    std::array const rawInputDevices{
        rid_t{
            .usUsagePage = 0x01,
            .usUsage = 0x06,
            .dwFlags = RIDEV_INPUTSINK,
            .hwndTarget = hwnd
        }
    };

    auto const result = RegisterRawInputDevices(rawInputDevices.data(), static_cast<UINT>(rawInputDevices.size()), sizeof(rid_t));

    CTH_STABLE_ERR(result == FALSE, "failed to register raw input devices, error: {}", GetLastError())
         throw details->exception();
}

}
