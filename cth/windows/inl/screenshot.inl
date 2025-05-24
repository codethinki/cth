#pragma once
#include "win_types.hpp"

namespace cth::win {

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