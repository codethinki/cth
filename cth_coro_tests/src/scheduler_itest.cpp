#include "test.hpp"

#include "cth/coro/sync/fence.hpp"

#include <cth/coro/scheduler.hpp>

#include <atomic>
#include <latch>
#include <barrier>
#include <chrono>
#include <future>
#include <semaphore>

namespace cth::co {

CORO_ITEST(scheduler, autostart_constructor_starts_immediately) {
    std::atomic<bool> executed{false};
    std::latch latch(1);

    scheduler sched(autostart, 1);

    // Should already be active
    EXPECT_TRUE(sched.active());

    sched.post(
        [&]() {
            executed.store(true, std::memory_order_release);
            latch.count_down();
        }
    );

    latch.wait();
    EXPECT_TRUE(executed.load());
    sched.await_stop();
}

// ========== Single Task Execution Tests ==========

CORO_ITEST(scheduler, executes_single_task) {
    scheduler sched(1);
    std::atomic<bool> executed{false};

    sched.start();
    sched.post([&]() { executed.store(true, std::memory_order_release); });

    // Wait for task execution with timeout
    auto start = std::chrono::steady_clock::now();
    while(!executed.load(std::memory_order_acquire)) {
        if(std::chrono::steady_clock::now() - start > std::chrono::milliseconds(100))
            FAIL() << "Task did not execute within timeout";

        std::this_thread::yield();
    }

    EXPECT_TRUE(executed.load());
    sched.await_stop();
}

CORO_ITEST(scheduler, executes_multiple_sequential_tasks) {
    scheduler sched(1);
    std::atomic<int> counter{0};
    constexpr int taskCount = 5;
    std::latch latch(taskCount);

    sched.start();
    for(int i = 0; i < taskCount; ++i) {
        sched.post(
            [&]() {
                counter.fetch_add(1, std::memory_order_relaxed);
                latch.count_down();
            }
        );
    }

    latch.wait();
    EXPECT_EQ(counter.load(), taskCount);
    sched.await_stop();
}

// ========== Multi-Worker Tests ==========

CORO_ITEST(scheduler, multiple_workers_execute_tasks) {
    constexpr size_t workers = 2;

    scheduler sched{workers};
    constexpr int taskCount = 2 * workers;

    std::barrier batchBarrier{workers};

    std::latch tasksStarted{taskCount};

    sched.start();

    for(int i = 0; i < taskCount; ++i)
        sched.post(
            [&batchBarrier, &tasksStarted]() {
                tasksStarted.count_down();
                batchBarrier.arrive_and_wait();
            }
        );

    tasksStarted.wait();
    sched.await_stop();
}



// ========== Task Posting Tests ==========

CORO_ITEST(scheduler, post_before_start_executes_after_start) {
    scheduler sched(1);
    std::atomic<bool> executed{false};
    std::latch latch(1);

    sched.post(
        [&]() {
            executed.store(true, std::memory_order_release);
            latch.count_down();
        }
    );
    EXPECT_FALSE(executed.load());

    sched.start();
    latch.wait();

    EXPECT_TRUE(executed.load());
}

CORO_ITEST(scheduler, post_during_execution) {
    scheduler sched(2);
    std::atomic<int> counter{0};
    constexpr int initialTasks = 3;
    constexpr int additionalTasks = 3;
    std::latch initialLatch(initialTasks);
    std::latch allLatch(initialTasks + additionalTasks);

    sched.start();

    // Post initial tasks
    for(int i = 0; i < initialTasks; ++i) {
        sched.post(
            [&]() {
                counter.fetch_add(1, std::memory_order_relaxed);
                initialLatch.count_down();
                allLatch.count_down();
            }
        );
    }

    // Wait for initial tasks, then post more
    initialLatch.wait();

    for(int i = 0; i < additionalTasks; ++i) {
        sched.post(
            [&]() {
                counter.fetch_add(1, std::memory_order_relaxed);
                allLatch.count_down();
            }
        );
    }

    allLatch.wait();
    EXPECT_EQ(counter.load(), initialTasks + additionalTasks);
}

CORO_ITEST(scheduler, owns_thread) {
    constexpr size_t taskCount = 2;

    std::atomic<int> count{0};
    std::barrier taskSync{taskCount + 1};

    scheduler sched{autostart, taskCount};

    ASSERT_FALSE(sched.owns_thread());

    for(int i = 0; i < taskCount; ++i) {
        sched.post(
            [&]() {
                if(sched.owns_thread()) ++count;
                taskSync.arrive_and_wait();
            }
        );
    }
    taskSync.arrive_and_wait();

    EXPECT_EQ(count.load(), taskCount);
}

// ========== Lifecycle Tests ==========

CORO_ITEST(scheduler, restart_cycle) {
    scheduler sched(2);
    std::atomic<int> counter{0};
    std::latch latch1(1);

    // First run
    sched.start();
    sched.post(
        [&]() {
            counter.fetch_add(1, std::memory_order_relaxed);
            latch1.count_down();
        }
    );
    latch1.wait();
    sched.await_stop();

    int firstCount = counter.load();
    EXPECT_EQ(firstCount, 1);

    // Second run
    std::latch latch2(1);
    sched.start();
    sched.post(
        [&]() {
            counter.fetch_add(1, std::memory_order_relaxed);
            latch2.count_down();
        }
    );
    latch2.wait();
    sched.await_stop();

    EXPECT_EQ(counter.load(), 2);
}

CORO_ITEST(scheduler, destructor_waits_for_active_tasks) {
    std::atomic taskCompleted{false};
    std::atomic destructorExited{false};


    fence taskStarted{};
    fence resumeTask{};

    std::jthread runner{
        [&] {
            {
                scheduler sched{autostart};
                sched.post(
                    [&] {
                        taskStarted.signal();
                        resumeTask.wait();
                        taskCompleted = true;
                    }
                );
                taskStarted.wait();
            }
            destructorExited = true;
        }
    };

    taskStarted.wait();

    EXPECT_FALSE(destructorExited);
    EXPECT_FALSE(taskCompleted);

    resumeTask.signal();
    runner.join();

    EXPECT_TRUE(destructorExited);
    EXPECT_TRUE(taskCompleted);
}
// ========== Stop Behavior Tests ==========

CORO_ITEST(scheduler, request_stop_prevents_new_task_execution) {
    scheduler sched{1};

    std::atomic counter{0};

    fence assertActive{};
    fence blockTaskBarrier{}; //main thread + task

    sched.start();

    sched.post(
        [&]() {
            assertActive.signal();
            blockTaskBarrier.wait();
        }
    );
    assertActive.wait();

    sched.request_stop();

    for(int i = 0; i < 10; ++i) { sched.post([&]() { ++counter; }); }

    blockTaskBarrier.signal();

    EXPECT_EQ(counter.load(), 0);
}

CORO_ITEST(scheduler, await_stop_completes_after_tasks_finish) {
    fence running{};
    fence completion{};

    scheduler sched{autostart, 1};
    sched.post(
        [&]() {
            running.signal();
            completion.wait();
        }
    );

    running.wait();

    fence waited{};
    std::jthread _{
        [&sched, &waited]() {
            sched.await_stop();
            waited.signal();
        }
    };

    EXPECT_FALSE(waited.signaled());

    completion.signal();

    waited.wait();

    EXPECT_FALSE(sched.active());
}


CORO_ITEST(scheduler, active_and_active_workers) {
    constexpr int taskCount = 4;

    scheduler sched(taskCount);

    std::barrier activeWorkersBarrier{2};
    std::latch mainThreadLatch{taskCount};
    std::barrier mainBarrier(taskCount + 1); // +1 for main thread


    sched.start();

    for(int i = 0; i < taskCount; ++i) {
        sched.post(
            [&]() {
                activeWorkersBarrier.arrive_and_wait();

                mainThreadLatch.count_down();

                mainBarrier.arrive_and_wait();
            }
        );
        activeWorkersBarrier.arrive_and_wait();

        ASSERT_TRUE(sched.active());
        ASSERT_EQ(sched.active_workers(), i + 1);
    }


    mainBarrier.arrive_and_wait();

    sched.await_stop();

    ASSERT_FALSE(sched.active());
    ASSERT_EQ(sched.active_workers(), 0);
}


// ========== Edge Cases ==========

CORO_ITEST(scheduler, many_small_tasks) {
    scheduler sched(2);
    constexpr int taskCount = 100;
    std::atomic<int> counter{0};
    std::latch latch(taskCount);

    sched.start();

    for(int i = 0; i < taskCount; ++i) {
        sched.post(
            [&]() {
                counter.fetch_add(1, std::memory_order_relaxed);
                latch.count_down();
            }
        );
    }

    latch.wait();
    EXPECT_EQ(counter.load(), taskCount);

    sched.await_stop();
}

CORO_ITEST(scheduler, task_chain_with_dependencies) {
    scheduler sched(1);
    std::atomic<int> executionOrder{0};
    std::array<int, 3> order{};
    std::latch latch(1);

    sched.start();

    // Chain of tasks that must execute in order on single worker
    sched.post([&]() { order[0] = executionOrder.fetch_add(1, std::memory_order_relaxed); });

    sched.post([&]() { order[1] = executionOrder.fetch_add(1, std::memory_order_relaxed); });

    sched.post(
        [&]() {
            order[2] = executionOrder.fetch_add(1, std::memory_order_relaxed);
            latch.count_down();
        }
    );

    latch.wait();

    // With single worker, tasks should execute in order
    EXPECT_EQ(order[0], 0);
    EXPECT_EQ(order[1], 1);
    EXPECT_EQ(order[2], 2);

    sched.await_stop();
}

}
