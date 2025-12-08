#include "test.hpp"
#include "cth/coro/fence.hpp"
#include <cth/coro/scheduler.hpp>

#include <atomic>
#include <latch>
#include <barrier>
#include <thread>

namespace cth::co {

CORO_TEST(scheduler, default_constructor) {
    scheduler sched;
    EXPECT_EQ(sched.workers(), 1);
    EXPECT_EQ(sched.active_workers(), 0);
    EXPECT_FALSE(sched.active());
}

CORO_TEST(scheduler, worker_count_constructor) {
    scheduler sched(4);
    EXPECT_EQ(sched.workers(), 4);
}

CORO_TEST(scheduler, autostart_constructor) {
    scheduler sched(autostart, 2);
    EXPECT_EQ(sched.workers(), 2);
    EXPECT_TRUE(sched.active());
}

CORO_TEST(scheduler, active_state) {
    scheduler sched(1);
    EXPECT_FALSE(sched.active());

    sched.start();
    EXPECT_TRUE(sched.active());

    sched.request_stop();
    sched.await_stop();
    EXPECT_FALSE(sched.active());
}

CORO_TEST(scheduler, owns_thread) {
    scheduler sched;
    EXPECT_FALSE(sched.owns_thread());

    sched.start();
    EXPECT_FALSE(sched.owns_thread());
}

CORO_TEST(scheduler, single_task) {
    scheduler sched(1);
    fence done;

    sched.start();
    sched.post([&] { done.signal(); });

    done.wait();
}

CORO_TEST(scheduler, sequential_tasks) {
    scheduler sched(1);
    std::atomic<int> counter{0};
    constexpr int taskCount = 5;
    std::latch allDone(taskCount);

    sched.start();
    for(int i = 0; i < taskCount; ++i) {
        sched.post(
            [&] {
                counter.fetch_add(1, std::memory_order_relaxed);
                allDone.count_down();
            }
        );
    }

    allDone.wait();
    EXPECT_EQ(counter.load(), taskCount);
}

CORO_TEST(scheduler, concurrent_execution) {
    constexpr size_t workers = 2;
    scheduler sched{workers};

    std::barrier workerSync{workers};
    std::latch tasksDone{workers};

    sched.start();

    for(size_t i = 0; i < workers; ++i) {
        sched.post(
            [&] {
                workerSync.arrive_and_wait();
                tasksDone.count_down();
            }
        );
    }

    tasksDone.wait();
}

CORO_TEST(scheduler, post_before_start) {
    scheduler sched(1);
    fence done;

    sched.post([&] { done.signal(); });
    EXPECT_FALSE(done.signaled());

    sched.start();
    done.wait();
}

CORO_TEST(scheduler, post_during_execution) {
    scheduler sched(2);
    constexpr int batch1 = 3;
    constexpr int batch2 = 3;

    std::latch batch1Done(batch1);
    std::latch batch2Done(batch2);
    std::atomic<int> counter{0};

    sched.start();

    for(int i = 0; i < batch1; ++i) {
        sched.post(
            [&] {
                counter.fetch_add(1, std::memory_order_relaxed);
                batch1Done.count_down();
            }
        );
    }

    batch1Done.wait();

    for(int i = 0; i < batch2; ++i) {
        sched.post(
            [&] {
                counter.fetch_add(1, std::memory_order_relaxed);
                batch2Done.count_down();
            }
        );
    }

    batch2Done.wait();
    EXPECT_EQ(counter.load(), batch1 + batch2);
}

CORO_TEST(scheduler, owns_thread_in_task) {
    constexpr int taskCount = 4;
    scheduler sched{autostart, 2};

    std::atomic<int> successCount{0};
    std::latch allDone{taskCount};

    for(int i = 0; i < taskCount; ++i) {
        sched.post(
            [&] {
                if(sched.owns_thread()) { successCount.fetch_add(1, std::memory_order_relaxed); }
                allDone.count_down();
            }
        );
    }

    allDone.wait();
    EXPECT_EQ(successCount.load(), taskCount);
}

CORO_TEST(scheduler, task_ordering_single_worker) {
    scheduler sched(1);
    int val = 0;
    fence done;

    sched.start();

    sched.post([&] { val = 1; });
    sched.post([&] { val = 2; });
    sched.post(
        [&] {
            val = 3;
            done.signal();
        }
    );

    done.wait();
    EXPECT_EQ(val, 3);
}

CORO_TEST(scheduler, restart) {
    scheduler sched(1);
    int counter = 0;

    {
        fence done;
        sched.start();
        sched.post(
            [&] {
                counter++;
                done.signal();
            }
        );
        done.wait();
        sched.await_stop();
    }
    EXPECT_EQ(counter, 1);

    {
        fence done;
        sched.start();
        sched.post(
            [&] {
                counter++;
                done.signal();
            }
        );
        done.wait();
    }
    EXPECT_EQ(counter, 2);
}

CORO_TEST(scheduler, destructor_waits) {
    std::atomic<bool> taskDone{false};
    std::atomic<bool> dtorDone{false};

    fence taskStarted;
    fence allowFinish;

    std::jthread runner(
        [&] {
            {
                scheduler sched{autostart};
                sched.post(
                    [&] {
                        taskStarted.signal();
                        allowFinish.wait();
                        taskDone = true;
                    }
                );
                taskStarted.wait();
            }
            dtorDone = true;
        }
    );

    taskStarted.wait();

    EXPECT_FALSE(dtorDone);
    EXPECT_FALSE(taskDone);

    allowFinish.signal();
}

CORO_TEST(scheduler, stop_prevents_new_tasks) {
    scheduler sched{1};
    std::atomic<int> counter{0};

    fence taskRunning;
    fence allowTaskFinish;

    sched.start();

    sched.post(
        [&] {
            taskRunning.signal();
            allowTaskFinish.wait();
        }
    );
    taskRunning.wait();

    sched.request_stop();

    for(int i = 0; i < 10; ++i) { sched.post([&] { ++counter; }); }

    allowTaskFinish.signal();
    sched.await_stop();

    EXPECT_EQ(counter.load(), 0);
}

CORO_TEST(scheduler, await_stop_waits) {
    scheduler sched{autostart, 1};

    fence taskRunning;
    fence allowTaskFinish;
    fence awaitStopReturned;

    sched.post(
        [&] {
            taskRunning.signal();
            allowTaskFinish.wait();
        }
    );

    taskRunning.wait();

    [[maybe_unused]] std::jthread stopper(
        [&] {
            sched.await_stop();
            awaitStopReturned.signal();
        }
    );

    EXPECT_FALSE(awaitStopReturned.signaled());

    allowTaskFinish.signal();

    awaitStopReturned.wait();
    EXPECT_FALSE(sched.active());
}

CORO_TEST(scheduler, active_worker_count) {
    constexpr int taskCount = 4;
    scheduler sched(taskCount);
    std::barrier stepSync(2);
    std::latch mainLatch{taskCount + 1};

    sched.start();

    for(int i = 0; i < taskCount; ++i) {
        sched.post(
            [&] {
                stepSync.arrive_and_wait();
                stepSync.arrive_and_wait();
                mainLatch.arrive_and_wait();
            }
        );

        stepSync.arrive_and_wait();

        EXPECT_TRUE(sched.active());
        EXPECT_GE(sched.active_workers(), i + 1);

        stepSync.arrive_and_wait();
    }
    mainLatch.count_down();

    sched.await_stop();
    EXPECT_EQ(sched.active_workers(), 0);
}

CORO_TEST(scheduler, move_semantics) {
    scheduler sched1(2);
    scheduler sched2(std::move(sched1));
    EXPECT_EQ(sched2.workers(), 2);

    scheduler sched3(1);
    sched3 = std::move(sched2);
    EXPECT_EQ(sched3.workers(), 2);
}

CORO_TEST(scheduler, non_copyable) {
    EXPECT_FALSE(std::is_copy_constructible_v<scheduler>);
    EXPECT_FALSE(std::is_copy_assignable_v<scheduler>);
}

CORO_TEST(scheduler, many_small_tasks) {
    scheduler sched(2);
    constexpr int taskCount = 100;
    std::atomic<int> counter{0};
    std::latch latch(taskCount);

    sched.start();

    for(int i = 0; i < taskCount; ++i)
        sched.post(
            [&]() {
                counter.fetch_add(1, std::memory_order_relaxed);
                latch.count_down();
            }
        );

    latch.wait();
    EXPECT_EQ(counter.load(), taskCount);

    sched.await_stop();
}


} // namespace cth::co
