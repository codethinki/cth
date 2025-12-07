#pragma once
#include <coroutine>
#include <utility>

namespace cth::co::dev {

//TODO add documentation
//TODO clean up / extend the implementation to fit standards

template<class Promise>
class unique_cohandle {
public:
    constexpr unique_cohandle() noexcept : _handle(nullptr) {}
    explicit unique_cohandle(std::coroutine_handle<Promise> h) noexcept : _handle(h) {}

    ~unique_cohandle() { optDestroy(); }

    [[nodiscard]] auto extract() noexcept { return std::exchange(_handle, nullptr); }

    void swap(unique_cohandle& other) noexcept { std::swap(_handle, other._handle); }

    friend void swap(unique_cohandle& a, unique_cohandle& b) noexcept { a.swap(b); }

protected:
    void optDestroy() { if(_handle) _handle.destroy(); }


    std::coroutine_handle<Promise> _handle;

public:
    [[nodiscard]] bool done() const noexcept { return !_handle || _handle.done(); }

    void resume() { if(!done()) _handle.resume(); }

    [[nodiscard]] auto get(this auto&& self) noexcept { return self._handle; }


    explicit operator bool() const noexcept { return static_cast<bool>(_handle); }



    unique_cohandle(unique_cohandle const&) = delete;
    unique_cohandle& operator=(unique_cohandle const&) = delete;

    unique_cohandle(unique_cohandle&& other) noexcept
        : _handle(std::exchange(other._handle, nullptr)) {}

    unique_cohandle& operator=(unique_cohandle&& other) noexcept {
        other.swap(*this);
        return *this;
    }
};
}
