#pragma once
#include <coroutine>
namespace cth::co {
class executor;
}

namespace cth::co {

namespace this_coro {
    struct get_executor_t {};
    inline constexpr get_executor_t executor{};
}

class executor_awaiter {
public:
    executor_awaiter(executor& exec) : _exec(&exec) {}

    bool await_ready() const noexcept { return true; }
    void await_suspend(std::coroutine_handle<>) const noexcept {}
    executor& await_resume() const noexcept { return *_exec; }

private:
    executor* _exec;
};

}

