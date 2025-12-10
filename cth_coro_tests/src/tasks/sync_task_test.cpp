#include "cth/coro/tasks/sync_task.hpp"

#include "test.hpp"

namespace cth::co {
CORO_TEST(sync_task, blocks_until_finished) {
    fence coroStarted{};
    fence allowClose{};

    auto task = [](fence& started, fence const& close) -> sync_task<void> {
        started.signal();
        close.wait();
        co_return;
    }(coroStarted, allowClose);

    std::jthread runner{[&task]() { task.handle().resume(); }};

    coroStarted.wait();

    EXPECT_FALSE(task.handle().done());

    fence awaiterStarted{};
    fence finished{}; //technically can't verify behavior, hopefully at least flaky

    std::jthread awaiter{
        [&task, &awaiterStarted, &finished]() {
            awaiterStarted.signal();
            task.handle().promise().wait();
            finished.signal();
        }
    };

    awaiterStarted.wait();
    EXPECT_FALSE(finished.signaled());

    allowClose.signal();
    runner.join();
    finished.wait();
}
}
