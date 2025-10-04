#pragma once

#include "cth/exception.hpp"
#include "cth/io/log.hpp"

#include <memory>



namespace cth::except {
class win_exception : public default_exception {
public:
    win_exception(
        std::string_view msg,
        Severity severity,
        std::source_location const& loc,
        std::stacktrace trace
    );
};
}

#define CTH_STABLE_WIN_ERR(expression, fmt_message, ...) \
    CTH_STABLE_ERR_T(cth::except::win_exception, expression, fmt_message, __VA_ARGS__)

#define CTH_STABLE_WIN_THROW(expression, fmt_message, ...) \
    CTH_STABLE_THROW_T(\
        cth::except::win_exception,\
        expression,\
        fmt_message,\
        __VA_ARGS__\
    )

namespace cth::win {
using dword_t = unsigned long;

using rect_t = struct {
    uint32_t x, y, width, height;
};

struct global_lock_deleter {
    void operator()(void* ptr) const;
};

using global_lock = std::unique_ptr<void, global_lock_deleter>;


struct bmp_deleter {
    void operator()(void* ptr) const;
};
using bmp_ptr = std::unique_ptr<void*, bmp_deleter>;

struct hdc_deleter {
    void operator()(void* ptr) const;
};
using hdc_ptr = std::unique_ptr<void, hdc_deleter>;

struct hwnd_deleter {
    void operator()(void* handle) const;
};

struct file_deleter {
    void operator()(void* handle) const;
};


using file_ptr = std::unique_ptr<void, file_deleter>;

using hwnd_ptr = std::unique_ptr<void, hwnd_deleter>;

struct window_class_t;

void swap(window_class_t& a, window_class_t& b) noexcept;

struct window_class_t {
    window_class_t(void* const h_instance, std::wstring id) : hInstance{h_instance}, id{std::move(id)} {}
    void* hInstance;
    std::wstring id;

    std::wstring release() { return std::exchange(id, {}); }

    ~window_class_t() noexcept;
    window_class_t(window_class_t const& other) = delete;
    window_class_t& operator=(window_class_t const& other) = delete;
    window_class_t(window_class_t&& other) noexcept { swap(*this, other); }
    window_class_t& operator=(window_class_t&& other) noexcept {
        swap(*this, other);
        return *this;
    }
};

inline void swap(window_class_t& a, window_class_t& b) noexcept {
    std::swap(a.hInstance, b.hInstance);
    std::swap(a.id, b.id);
}



struct window_t {
    std::optional<window_class_t> classOpt;
    hwnd_ptr handle;
};


struct monitor_t {
    /**
     * corresponds to @ref HMONITOR
     */
    void* handle;
    rect_t rect;
};
}
