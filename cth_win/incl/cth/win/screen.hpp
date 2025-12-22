#pragma once
#include "win_types.hpp"

#include <cth/io/log.hpp>

#include <span>



namespace cth::win {
hwnd_t desktop_handle();


/**
 * creates a windows window
 * @param name of window (in utf8), must not be empty
 * @param rect pos + size
 * @param class_name (in utf8) if empty => name is used, class doesn't exist => created
 * @throws cth::except::win_exception on window class registration failure
 * @throws cth::except::win_exception on window creation failure
 */
window_t create_window(std::string_view name, rect_t rect, bool visible = true, std::string_view class_name = {});


/**
 * Enumerates all monitors
 */
std::vector<monitor_t> enum_monitors();



/**
 * gets the rect of the window
 */
rect_t window_rect(hwnd_t hwnd);


/** 
 * Gets the desktop rect, origin top left
 */
inline rect_t desktop_rect() { return window_rect(desktop_handle()); }

}


namespace cth::win::screen {



/**
 * gives access to a section of the screen.
 * @note the color format is BGRA
 */
class section {
public:
    /**
     * in bytes
     */
    static constexpr size_t PIXEL_SIZE = 4;

    /**
     * constructs with pos & dimensions
     * @param rect pos and dimensions of section
     */
    section(rect_t rect);
    ~section();

    /**
     * reads the stored pixels
     * @return @ref view()
     * @throws cth::except::win_exception if BitBlt fails
     */
    std::span<std::byte> capture() {
        write_bmp(_screenDc.get(), _rect.x, _rect.y, _memoryDc.get(), 0, 0, _rect.width, _rect.height);
        return view();
    }

    /**
     * writes the stored pixels
     * @throws cth::except::win_exception if BitBlt fails
     */
    void write() const {
        //TODO figure out why writes are not visible on screen
        write_bmp(_memoryDc.get(), 0, 0, _screenDc.get(), _rect.x, _rect.y, _rect.width, _rect.height);
        flush();
    }

    auto view(this auto&& s) { return std::span{s.raw(), s.bytes()}; }

    void relocate(ssize_t x, ssize_t y);

private:
    static void flush();

    static void write_bmp(
        void* src,
        ssize_t src_x,
        ssize_t src_y,
        void* dst,
        ssize_t dst_x,
        ssize_t dst_y,
        size_t width,
        size_t height
    );

    void* selectBmp(void* bmp);

    rect_t _rect;

    borrow_dc_ptr _screenDc;

    unique_dc_ptr _memoryDc{};
    bmp_ptr _bmp{};

    void* _oldBmp = nullptr;

    void* _data{};

public:
    size_t width() const { return _rect.width; }
    size_t height() const { return _rect.height; }

    size_t pixels() const { return width() * height(); }

    size_t bytes() const { return pixels() * PIXEL_SIZE; }

    std::byte* raw() { return static_cast<std::byte*>(data()); }
    std::byte const* raw() const { return static_cast<std::byte const*>(data()); }

    void* data() { return _data; }
    void const* data() const { return _data; }



};
}
