#pragma once
#include "cth/coro/utility.hpp"
#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/tasks/dev/capture_task.hpp"
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/utility/fwd.hpp"

#include "cth/meta/coro.hpp"

namespace cth::co {



template<this_coro::scheduler_payload Pyld, non_this_coro_awaitable Awaitable>
[[nodiscard]] auto steal(Awaitable awaitable, Pyld const& payload) -> capture_task<awaited_t<Awaitable>> {
    static_assert(has_scheduler<Pyld>, "foreign awaitables can only be stolen with a scheduler in the payload");

    using result_t = awaited_t<Awaitable>;

    if constexpr(std::same_as<void, result_t>) {
        co_await std::move(awaitable);
        co_await schedule_awaiter{payload.scheduler()};
        co_return;
    } else {
        decltype(auto) result = co_await std::move(awaitable);
        co_await schedule_awaiter{payload.scheduler()};
        co_return result;
    }
}


template<payload Pyld, this_coro_awaitable Awaitable>
constexpr auto steal(Awaitable&& awaitable, Pyld const& payload) -> awaiter_t<Awaitable> {
    static_assert(this_coro_awaitable_with<Awaitable, Pyld>, "awaitable not compatible with payload type");

    auto awaiter = co::extract_awaiter(std::forward<Awaitable>(awaitable));
    awaiter.inject(payload);

    return awaiter;
}


template<payload Pyld, this_coro_awaitable Awaitable, class... Args>
constexpr auto steal(Awaitable&& awaitable, Args&&... args) -> awaiter_t<Awaitable> {
    return co::steal(std::forward<Awaitable>(awaitable), Pyld{std::forward<Args>(args)...});
}

template<payload Pyld, non_this_coro_awaitable Awaitable, class... Args>
[[nodiscard]] auto steal(Awaitable awaitable, Args&&... args) -> capture_task<awaited_t<Awaitable>> {
    return co::steal(std::move(awaitable), Pyld{std::forward<Args>(args)...});
}



template<non_this_coro_awaitable Awaitable>
[[nodiscard]] auto steal(
    Awaitable awaitable,
    scheduler const& scheduler
) -> capture_task<awaited_t<Awaitable>> {
    return co::steal(std::move(awaitable), this_coro::default_payload{scheduler});
}

template<this_coro_awaitable Awaitable>
constexpr auto steal(Awaitable&& awaitable, scheduler const& scheduler) -> awaiter_t<Awaitable> {
    return co::steal(std::forward<Awaitable>(awaitable), this_coro::default_payload{scheduler});
}


}
