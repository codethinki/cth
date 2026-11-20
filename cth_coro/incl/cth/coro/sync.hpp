#pragma once

#include "cth/coro/tasks/sync_task.hpp"


namespace cth::co {

/**
 * awaits the awaitable on the calling thread and blocks until it ends
 * @details the caller has no context: spawn `this_coro` tasks with `executor::spawn()`
 * @details `executor::sync_wait()` spawns and waits in one call
 * @param awaitable to await
 * @return result of the awaitable
 * @throws what the awaitable throws
 * @attention blocks the calling thread
 */
template<awaitable Awaitable>
auto sync(Awaitable awaitable) -> awaited_t<Awaitable> {
    using awaited_type = awaited_t<Awaitable>;

    auto syncTask = [](Awaitable a) -> sync_task<awaited_type> {
        co_return co_await a; // void: co_await, then co_return
    }(std::move(awaitable));

    syncTask.start();


    return syncTask.await();
}

}
