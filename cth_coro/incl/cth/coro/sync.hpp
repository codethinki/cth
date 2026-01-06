#pragma once

#include "cth/coro/tasks/sync_task.hpp"


namespace cth::co {

template<awaitable Awaitable>
auto sync(Awaitable awaitable) -> awaited_t<Awaitable> {
    using awaited_type = awaited_t<Awaitable>;

    auto syncTask = [](Awaitable a) -> sync_task<awaited_type> {
        if constexpr(type::is_void<awaited_t<Awaitable>>)
            co_await a;
        else co_return co_await a;
    }(std::move(awaitable));

    syncTask.resume();


    return syncTask.await();
}

}
