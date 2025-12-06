#pragma once
#include <coroutine>
#include <utility>

namespace cth::co::dev {

template<class Promise>
class unique_handle {
public:
    constexpr unique_handle() noexcept : _handle(nullptr) {}
    explicit unique_handle(std::coroutine_handle<Promise> h) noexcept : _handle(h) {}

    ~unique_handle() { optDestroy(); }

    [[nodiscard]] auto extract() noexcept { return std::exchange(_handle, nullptr); }

    void swap(unique_handle& other) noexcept { std::swap(_handle, other._handle); }

    friend void swap(unique_handle& a, unique_handle& b) noexcept { a.swap(b); }

protected:
    void optDestroy() { if(_handle) _handle.destroy(); }


    std::coroutine_handle<Promise> _handle;

public:
    [[nodiscard]] bool done() const noexcept { return !_handle || _handle.done(); }

    void resume() { if(!done()) _handle.resume(); }

    [[nodiscard]] auto get(this auto&& self) noexcept { return self._handle; }


    explicit operator bool() const noexcept { return static_cast<bool>(_handle); }



    unique_handle(unique_handle const&) = delete;
    unique_handle& operator=(unique_handle const&) = delete;

    unique_handle(unique_handle&& other) noexcept
        : _handle(std::exchange(other._handle, nullptr)) {}

    unique_handle& operator=(unique_handle&& other) noexcept {
        other.swap(*this);
        return *this;
    }
};
}
