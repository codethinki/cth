#pragma once
#include "cth/win/win_types.hpp"

#include <chrono>
#include <memory>
#include <span>


namespace cth::win::capture {

/**
 * checks if window capture is supported on this system
 * @details requires >= WIN10 1903
 */
bool supported();


/**
 * captures a window's buffer via Windows.Graphics.Capture
 * @note the color format is BGRA
 * @note occluded windows are captured, minimized windows produce no frames
 * @note the system may draw a border around the captured window
 */
class window_capture {
public:
    /**
     * in bytes
     */
    static constexpr size_t PIXEL_SIZE = 4;

    /**
     * begins capturing the window's buffer
     * @param window to capture, must not be minimized
     * @throws cth::except::win_exception if capture is not @ref supported()
     * @throws cth::except::win_exception on capture creation failure
     * @note initializes WinRT (MTA) on the calling thread if required
     */
    explicit window_capture(hwnd_t window);
    ~window_capture();

    /**
     * waits for the next frame and stores it
     * @param timeout max wait, 0 => infinite
     * @return @ref view(), empty on timeout or @ref closed()
     * @throws cth::except::win_exception on frame copy failure
     */
    std::span<std::byte> next_frame(std::chrono::milliseconds timeout = {});

    /**
     * stores the newest frame if one is available, never blocks
     * @return @ref view(), empty if no new frame
     * @throws cth::except::win_exception on frame copy failure
     */
    std::span<std::byte> try_frame();

    /**
     * enables / disables capturing the cursor
     * @details enabled by default
     */
    void capture_cursor(bool enable);

private:
    struct impl;
    std::unique_ptr<impl> _impl;

public:
    /**
     * @return true once the captured window was destroyed
     */
    bool closed() const;

    size_t width() const;
    size_t height() const;

    size_t pixels() const { return width() * height(); }

    size_t bytes() const { return pixels() * PIXEL_SIZE; }

    std::span<std::byte> view();
    std::span<std::byte const> view() const;

    window_capture(window_capture const&) = delete;
    window_capture& operator=(window_capture const&) = delete;
    window_capture(window_capture&&) noexcept;
    window_capture& operator=(window_capture&&) noexcept;
};

}
