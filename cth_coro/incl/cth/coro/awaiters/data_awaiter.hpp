#pragma once
#include <coroutine>


namespace cth::co {
template<class T>
class data_awaiter {
public:
    explicit data_awaiter(T type) : _type(type) {}

    bool await_ready() const noexcept { return true; }
    void await_suspend(std::coroutine_handle<>) const noexcept {}
    T await_resume() const noexcept { return _type; }

private:
    T _type;
};
}
