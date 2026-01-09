#pragma once
#include <coroutine>


namespace cth::co {
template<class T>
class data_awaiter {
public:
    constexpr explicit data_awaiter(T type) : _type{type} {}

    [[nodiscard]] constexpr bool await_ready() const noexcept { return true; }

    constexpr void await_suspend(std::coroutine_handle<>) const noexcept {}

    [[nodiscard]] constexpr T await_resume() const noexcept { return _type; }

private:
    T _type;
};

}
