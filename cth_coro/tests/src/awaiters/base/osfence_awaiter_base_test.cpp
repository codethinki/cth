#include "cth/coro/awaiters/base/osfence_awaiter_base.hpp"

#include "../../test.hpp"

#include <cth/coro/executor.hpp>
#include <cth/coro/sync.hpp>
#include <cth/coro/tasks/executor_task.hpp>

#include <atomic>
#include <chrono>
#include <thread>

namespace cth::co {
namespace {
struct test_osfence_awaiter : osfence_awaiter_base<> {
    using osfence_awaiter_base<>::osfence_awaiter_base;

    static constexpr void await_resume() noexcept {}
};

auto await_fence(os::fence const& fence, std::atomic_bool const& signaled) ->
    executor_task<bool> {
    co_await test_osfence_awaiter{fence};
    co_return signaled.load();
}
}

CORO_TEST(osfence_awaiter_base, ready_when_signaled) {
    os::fence fence{true};

    EXPECT_TRUE(test_osfence_awaiter{fence}.await_ready());
}

CORO_TEST(osfence_awaiter_base, suspends_until_signaled) {
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

    EXPECT_TRUE(sync(exec.spawn(await_fence(fence, signaled))));
}

}
