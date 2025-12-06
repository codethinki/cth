#pragma once
#include "executor_promise_base_decl.hpp"

#include "cth/coro/dev/executor_task_decl.hpp"

#include "executor.hpp"


namespace cth::co {
template<class Awaitable>
auto executor_promise_base::await_transform(Awaitable&& awaitable) -> capture_task<awaited_t<Awaitable>> {
    return exec->steal(std::forward<Awaitable>(awaitable));
}
}
