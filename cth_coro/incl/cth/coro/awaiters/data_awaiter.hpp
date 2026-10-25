#pragma once
#include <cth/meta/concepts.hpp>

#include <coroutine>
#include <utility>


namespace cth::co {
template<class T>
class data_awaiter {
public:
    constexpr explicit data_awaiter(T value) noexcept(mta::nothrow_constructible<T, T&&>) :
        _type{std::forward<T>(value)} {}

    [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }

    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}

    /**
     * @attention moves the value out, resume once
     */
    [[nodiscard]] constexpr T await_resume() noexcept(mta::nothrow_constructible<T, T&&>) {
        return std::forward<T>(_type);
    }

private:
    T _type;
};

}
