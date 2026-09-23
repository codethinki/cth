#include "cth/coro/awaiters/raw_native_handle_awaiter.hpp"

#include "../test.hpp"

#include <cth/coro/executor.hpp>
#include <cth/coro/sync.hpp>
#include <cth/coro/tasks/executor_task.hpp>
#include <cth/os/fence.hpp>

namespace cth::co {

CORO_TEST(native_handle_awaiter, direct_native_waitable) {
    os::fence fence;

    auto wait = [&fence]() -> executor_task<bool> {
        co_await fence;
        co_return fence.signaled();
    };

    scheduler scheduler{autostart, 1};
    auto result = executor{scheduler}.spawn(wait());
    fence.signal();

    EXPECT_TRUE(sync(std::move(result)));
}

}
