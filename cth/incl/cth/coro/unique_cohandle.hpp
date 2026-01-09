#pragma once
#include <coroutine>
#include <utility>
#include <cth/meta/coro.hpp>

namespace cth::co {

//TODO add documentation
//TODO clean up / extend the implementation to fit standards

template<promise Promise>
class unique_cohandle {
public:
    using promise_type = Promise;

    constexpr unique_cohandle() noexcept : _handle{nullptr} {}
    explicit constexpr unique_cohandle(std::coroutine_handle<Promise> h) noexcept : _handle{h} {}

    ~unique_cohandle() { optDestroy(); }

    [[nodiscard]] constexpr auto extract() noexcept { return std::exchange(_handle, nullptr); }

    constexpr void swap(unique_cohandle& other) noexcept { std::swap(_handle, other._handle); }

    friend constexpr void swap(unique_cohandle& a, unique_cohandle& b) noexcept { a.swap(b); }

protected:
    void optDestroy() { if(_handle) _handle.destroy(); }


    std::coroutine_handle<Promise> _handle;

public:
    [[nodiscard]] constexpr bool done() const noexcept { return !_handle || _handle.done(); }

    void resume() { if(!done()) _handle.resume(); }

    [[nodiscard]] constexpr auto get(this auto&& self) noexcept { return self._handle; }

    [[nodiscard]] Promise& promise() noexcept { return _handle.promise(); }


    explicit constexpr operator bool() const noexcept { return static_cast<bool>(_handle); }



    unique_cohandle(unique_cohandle const&) = delete;
    unique_cohandle& operator=(unique_cohandle const&) = delete;

    constexpr unique_cohandle(unique_cohandle&& other) noexcept
        : _handle(std::exchange(other._handle, nullptr)) {}

    constexpr unique_cohandle& operator=(unique_cohandle&& other) noexcept {
        other.swap(*this);
        return *this;
    }
};
}
