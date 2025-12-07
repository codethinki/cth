#pragma once
#include "cth/coro/tasks/dev/capture_task.hpp"

#include "cth/coro/awaiters/schedule_awaiter.hpp"

#include "cth/coro/utility/fwd.hpp"
#include "cth/types/coro.hpp"

namespace cth::co {


template<class Awaitable>
auto steal(
    scheduler& scheduler,
    Awaitable awaitable
) -> capture_task<awaited_t<Awaitable>> {
    using result_t = awaited_t<Awaitable>;

    if constexpr(std::same_as<void, result_t>) {
        co_await awaitable;
        co_await schedule_awaiter{scheduler};
        co_return;
    } else {
        auto result = co_await awaitable;
        co_await schedule_awaiter{scheduler};
        co_return result;
    }
}

}
