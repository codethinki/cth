#pragma once
#include <coroutine>

#include "cth/coro/utility/fwd.hpp"

namespace cth::co {

namespace this_coro {
    struct get_executor_t {};
    inline constexpr get_executor_t executor{};
}

class current_executor_awaiter {
public:
    current_executor_awaiter(executor& exec) : _exec(exec) {}

    bool await_ready() const noexcept { return true; }
    void await_suspend(std::coroutine_handle<>) const noexcept {}
    executor& await_resume() const noexcept { return _exec; }

private:
    executor& _exec;
};

}
