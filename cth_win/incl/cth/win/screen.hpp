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
window_t
create_window(std::string_view name, rect_t rect, bool visible = true, std::string_view class_name = {});


/**
 * Enumerates all monitors
 */
std::vector<monitor_t> enum_monitors();


}


namespace cth::win::screen {
enum class DpiAwareness {
    NONE,
    GDI_SCALED,
    /**
     * @details requires >= WIN10 1703
     */
    PER_MONITOR,
    PER_MONITOR_LEGACY,
};

/**
 * sets the windows dpi scaling behavior
 * @throws cth::except::win_exception on failure
 * @return previous awareness
 */
DpiAwareness set_thread_dpi_awareness(DpiAwareness awareness);


struct scoped_dpi_awareness {
    explicit scoped_dpi_awareness(DpiAwareness new_awareness) :
        _awareness{set_thread_dpi_awareness(new_awareness)} {}
    ~scoped_dpi_awareness() noexcept { set_thread_dpi_awareness(_awareness); }

private:
    DpiAwareness _awareness;
};

/**
 * gets the rect of the window
 * @throws cth::except::win_exception on failure
 */
rect_t window_rect(hwnd_t hwnd);


/**
 * Gets the desktop rect, origin top left
 * @details calls @ref window_rect(hwnd_t)
 */
inline rect_t desktop_rect() { return window_rect(desktop_handle()); }


/**
 * gets the frame count of a window
 * @param hwnd to check frame count of
 * @throws cth::except::win_exception on failure
 */
size_t window_frame_count(hwnd_t hwnd);

/**
 * gets the desktop frame count
 * @details calls @ref window_frame_count(hwnd_t)
 */
inline size_t desktop_frame_count() { return window_frame_count(desktop_handle()); }
}


namespace cth::win::screen {
double window_scale(hwnd_t window);

inline double desktop_scale() { return window_scale(desktop_handle()); }

/**
 * blits from src to dst
 * @param src for blit
 * @param dst for blit
 * @throws
 */
void raw_blit(
    hbmp_t src,
    ssize_t src_x,
    ssize_t src_y,
    hbmp_t dst,
    ssize_t dst_x,
    ssize_t dst_y,
    size_t width,
    size_t height
);

/**
 * blits from src to dst with scaling, falls back to raw_blit if sizes are equal
 * @param src to blit from
 * @param dst to blit to
 */
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
);

/**
 * blits a bmp to the screen
 * @param src buffer
 * @param dst screen
 * @param dpi_aware true => raw blit
 */
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
);

/**
 * blits from the screen to the buffer
 * @param src screen
 * @param dst buffer
 * @param dpi_aware true => raw blit
 */
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
);
}


namespace cth::win::screen {


inline void make_rgba_opaque(std::span<std::byte> pixel_fragments) {
    CTH_CRITICAL(pixel_fragments.size() % 4 != 0, "illegal pixel fragments") {}
    using pixel = std::byte[4];

    std::span<pixel> pixels{reinterpret_cast<pixel*>(pixel_fragments.data()), pixel_fragments.size() / 4};

#pragma omp simd
    for(auto& pixel : pixels)
        pixel[3] = std::byte{0xff};
}

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
     * @param dpi_aware if true, uses faster BitBlt instead of StretchBlt
     */
    section(rect_t rect, bool dpi_aware = false);
    ~section();

    /**
     * refreshes the stored pixels
     * @return @ref view()
     * @throws cth::except::win_exception if BitBlt fails
     * @attention blocks if captured faster than screen refresh rate
     */
    std::span<std::byte> capture() {
        blit_from_screen(
            _screenDc.get(),
            _rect.x,
            _rect.y,
            _memoryDc.get(),
            0,
            0,
            _rect.width,
            _rect.height,
            _dpiAware
        );
        make_rgba_opaque({raw(), bytes()});
        return view();
    }

    /**
     * writes the stored pixels
     * @throws cth::except::win_exception if BitBlt fails
     */
    void write() const {
        // TODO figure out why writes are not visible on screen
        blit_to_screen(
            _memoryDc.get(),
            0,
            0,
            _screenDc.get(),
            _rect.x,
            _rect.y,
            _rect.width,
            _rect.height,
            _dpiAware
        );
        flush();
    }


    void relocate(ssize_t x, ssize_t y);

private:
    static void flush();

    void* selectBmp(void* bmp);

    rect_t _rect;
    bool _dpiAware;

    borrow_dc_ptr _screenDc;

    unique_dc_ptr _memoryDc{};
    unique_bmp_ptr _bmp{};

    void* _oldBmp = nullptr;

    void* _data{};

public:
    rect_t rect() const { return _rect; }

    size_t width() const { return _rect.width; }
    size_t height() const { return _rect.height; }

    size_t pixels() const { return width() * height(); }

    size_t bytes() const { return pixels() * PIXEL_SIZE; }

    std::byte* raw() { return static_cast<std::byte*>(data()); }
    std::byte const* raw() const { return static_cast<std::byte const*>(data()); }

    void* data() { return _data; }
    void const* data() const { return _data; }

    template<class S>
    [[nodiscard]] auto view(this S&& s) -> std::span<mta::fwd_const_t<S, std::byte>> {
        return std::span{s.raw(), s.bytes()};
    }
};
}
