#include "cth/coro/awaiters/base/native_handle_awaiter_base.hpp"

#include "../../test.hpp"

#include <cth/coro/executor.hpp>
#include <cth/coro/sync.hpp>
#include <cth/coro/tasks/executor_task.hpp>
#include <cth/os/fence.hpp>

#include <atomic>
#include <chrono>
#include <thread>

namespace cth::co {
namespace {
struct test_native_handle_awaiter : native_handle_awaiter_base<> {
    using native_handle_awaiter_base<>::native_handle_awaiter_base;

    static constexpr void await_resume() noexcept {}
};

auto await_handle(os::native_handle_t handle, std::atomic_bool const& signaled) ->
    executor_task<bool> {
    co_await test_native_handle_awaiter{handle};
    co_return signaled.load();
}
}

CORO_TEST(native_handle_awaiter_base, ready_when_signaled) {
    os::fence fence{true};

    EXPECT_TRUE(test_native_handle_awaiter{fence.native_handle()}.await_ready());
}

CORO_TEST(native_handle_awaiter_base, suspends_until_signaled) {
    os::fence fence;
    std::atomic_bool signaled = false;
    scheduler sched{autostart, 1};
    executor exec{sched};

    std::jthread const signaller{
        [&fence, &signaled] {
            std::this_thread::sleep_for(std::chrono::milliseconds{25});
            signaled = true;
            fence.signal();
        }
    };

    EXPECT_TRUE(sync(exec.spawn(await_handle(fence.native_handle(), signaled))));
}

}
