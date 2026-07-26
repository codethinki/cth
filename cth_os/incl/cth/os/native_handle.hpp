#pragma once
#include <cth/os/osdef.hpp>

#include <utility>

#ifdef CTH_FS_WINDOWS
#define CTH_OS_WIN_POSIX_VALUE(win, posix) win
#elifdef CTH_FS_POSIX
#define CTH_OS_WIN_POSIX_VALUE(win, posix) posix
#else
#error "unsupported platform"
#endif


namespace cth::os {

/**
 * the platform's waitable handle
 * @details win: win32 HANDLE, linux: a posix file descriptor
 */
using native_handle_t = CTH_OS_WIN_POSIX_VALUE(void*, int);

/**
 * the value a handle carries when it owns nothing
 * @attention nonzero on posix
 */
[[nodiscard]] constexpr native_handle_t invalid_handle() noexcept {
    return CTH_OS_WIN_POSIX_VALUE(nullptr, -1);
}

/**
 * closes the handle
 * @details noop for @ref invalid_handle()
 * @return true if a handle was closed
 */
bool close_handle(native_handle_t handle) noexcept;

/**
 * duplicates the handle
 * @return the duplicate, owned by the caller
 * @throws cth::except::os_exception if the duplication fails
 */
[[nodiscard]] native_handle_t duplicate_handle(native_handle_t handle);


/**
 * owning @ref native_handle_t
 */
class unique_native_handle {
public:
    constexpr unique_native_handle() noexcept = default;
    explicit constexpr unique_native_handle(native_handle_t handle) noexcept : _handle{handle} {}

    ~unique_native_handle() { close_handle(_handle); }

    /**
     * closes the owned handle and takes @p handle in its place
     */
    void reset(native_handle_t handle = invalid_handle()) noexcept {
        close_handle(std::exchange(_handle, handle));
    }

    /**
     * gives up ownership without closing
     */
    [[nodiscard]] constexpr native_handle_t release() noexcept {
        return std::exchange(_handle, invalid_handle());
    }

private:
    native_handle_t _handle{invalid_handle()};

public:
    [[nodiscard]] constexpr native_handle_t get() const noexcept { return _handle; }

    /**
     * @return true if a handle is owned
     */
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return _handle != invalid_handle(); }

    unique_native_handle(unique_native_handle const&) = delete;
    unique_native_handle& operator=(unique_native_handle const&) = delete;

    constexpr unique_native_handle(unique_native_handle&& other) noexcept : _handle{other.release()} {}

    unique_native_handle& operator=(unique_native_handle&& other) noexcept {
        reset(other.release());
        return *this;
    }
};

}
