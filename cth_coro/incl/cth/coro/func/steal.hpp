#pragma once
#include "cth/coro/utility.hpp"
#include "cth/coro/tasks/dev/capture_task.hpp"

#include "cth/coro/awaiters/schedule_awaiter.hpp"

#include "cth/coro/utility/fwd.hpp"
#include "cth/types/coro.hpp"

namespace cth::co {


template<non_this_coro_awaitable Awaitable>
[[nodiscard]] auto steal(
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

template<this_coro_awaitable Awaitable>
auto steal(scheduler& scheduler, Awaitable&& awaitable) -> awaiter_t<Awaitable> {
    auto awaiter = co::extract_awaiter(std::forward<Awaitable>(awaitable));
    awaiter.inject(this_coro::payload{scheduler});;
    return awaiter;
}


}
