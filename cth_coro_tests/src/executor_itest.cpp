//#include "test.hpp"
//
//#include <cth/coro/executor.hpp>
//#include <cth/coro/scheduler.hpp>
//#include <cth/coro/executor_task.hpp>
//
//#include <atomic>
//#include <latch>
//#include <chrono>
//
//namespace cth::co {
//
//// ========== Helper Coroutines ==========
//namespace {
//    executor_void_task simple_task() { co_return; }
//
//    executor_void_task signal_task(std::atomic<bool>& flag) {
//        flag.store(true, std::memory_order_release);
//        co_return;
//    }
//
//    executor_task<int> value_task(int value) { co_return value; }
//
//    executor_void_task scheduling_task(executor exec, std::atomic<int>& counter) {
//        counter.fetch_add(1, std::memory_order_relaxed);
//        co_await exec.schedule();
//        counter.fetch_add(1, std::memory_order_relaxed);
//        co_return;
//    }
//}
//
//// ========== Schedule Tests ==========
//
//CORO_ITEST(executor, schedule_posts_to_scheduler) {
//    scheduler sched(autostart, 1);
//    executor exec(sched);
//    std::atomic<int> counter{0};
//    std::latch latch(1);
//
//    auto task = [&]() -> executor_void_task {
//        counter.fetch_add(1, std::memory_order_relaxed);
//        co_await exec.schedule();
//        counter.fetch_add(1, std::memory_order_relaxed);
//        latch.count_down();
//        co_return;
//    };
//
//    sched.post([t = task()]() mutable {});
//
//    latch.wait();
//    EXPECT_EQ(counter.load(), 2);
//    sched.await_stop();
//}
//
//CORO_ITEST(executor, schedule_skips_when_owns_thread) {
//    scheduler sched(autostart, 1);
//    executor exec(sched);
//    std::atomic<bool> schedulerThreadDetected{false};
//    std::latch latch(1);
//
//    sched.post([&]() {
//        auto task = [&]() -> executor_void_task {
//            // Should skip scheduling since we're already on scheduler thread
//            co_await exec.schedule();
//            schedulerThreadDetected.store(sched.owns_thread(), std::memory_order_release);
//            latch.count_down();
//            co_return;
//        };
//        auto t = task();
//    });
//
//    latch.wait();
//    EXPECT_TRUE(schedulerThreadDetected.load());
//    sched.await_stop();
//}
//
//// ========== Spawn Tests ==========
//
//CORO_ITEST(executor, spawn_executes_void_task) {
//    scheduler sched(autostart, 1);
//    executor exec(sched);
//    std::atomic<bool> executed{false};
//    std::latch latch(1);
//
//    auto inner_task = [&]() -> executor_void_task {
//        executed.store(true, std::memory_order_release);
//        co_return;
//    };
//
//    sched.post([&, t = exec.spawn(inner_task())]() mutable { latch.count_down(); });
//
//    latch.wait();
//    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Give time for task to execute
//    EXPECT_TRUE(executed.load());
//    sched.await_stop();
//}
//
//CORO_ITEST(executor, spawn_executes_value_task) {
//    scheduler sched(autostart, 1);
//    executor exec(sched);
//    std::atomic<bool> executed{false};
//    std::latch latch(1);
//
//    auto inner_task = [&]() -> executor_task<int> {
//        executed.store(true, std::memory_order_release);
//        co_return 42;
//    };
//
//    sched.post([&, t = exec.spawn(inner_task())]() mutable { latch.count_down(); });
//
//    latch.wait();
//    std::this_thread::sleep_for(std::chrono::milliseconds(10));
//    EXPECT_TRUE(executed.load());
//    sched.await_stop();
//}
//
//CORO_ITEST(executor, spawn_schedules_task_to_executor) {
//    scheduler sched(autostart, 2);
//    executor exec(sched);
//    std::atomic<int> counter{0};
//    std::latch latch(1);
//
//    auto inner_task = [&]() -> executor_void_task {
//        counter.fetch_add(1, std::memory_order_relaxed);
//        co_return;
//    };
//
//    auto outer_task = [&]() -> executor_void_task {
//        co_await exec.spawn(inner_task());
//        counter.fetch_add(1, std::memory_order_relaxed);
//        latch.count_down();
//        co_return;
//    };
//
//    sched.post([t = outer_task()]() mutable {});
//
//    latch.wait();
//    EXPECT_EQ(counter.load(), 2);
//    sched.await_stop();
//}
//
//// ========== Steal Tests ==========
//
//CORO_ITEST(executor, steal_wraps_foreign_awaitable) {
//    scheduler sched(autostart, 1);
//    executor exec(sched);
//    std::atomic<bool> executed{false};
//    std::latch latch(1);
//
//    auto foreign_task = [&]() -> executor_void_task {
//        executed.store(true, std::memory_order_release);
//        co_return;
//    };
//
//    auto wrapper_task = [&]() -> executor_void_task {
//        co_await exec.steal(foreign_task());
//        latch.count_down();
//        co_return;
//    };
//
//    sched.post([t = wrapper_task()]() mutable {});
//
//    latch.wait();
//    EXPECT_TRUE(executed.load());
//    sched.await_stop();
//}
//
//CORO_ITEST(executor, steal_returns_value_from_task) {
//    scheduler sched(autostart, 1);
//    executor exec(sched);
//    std::atomic<int> result{0};
//    std::latch latch(1);
//
//    auto value_producing_task = []() -> executor_task<int> { co_return 42; };
//
//    auto wrapper_task = [&]() -> executor_void_task {
//        int value = co_await exec.steal(value_producing_task());
//        result.store(value, std::memory_order_release);
//        latch.count_down();
//        co_return;
//    };
//
//    sched.post([t = wrapper_task()]() mutable {});
//
//    latch.wait();
//    EXPECT_EQ(result.load(), 42);
//    sched.await_stop();
//}
//
//CORO_ITEST(executor, steal_forwards_executor_awaitable_unchanged) {
//    scheduler sched(autostart, 1);
//    executor exec(sched);
//    std::atomic<bool> executed{false};
//    std::latch latch(1);
//
//    auto exec_task = [&]() -> executor_void_task {
//        executed.store(true, std::memory_order_release);
//        co_return;
//    };
//
//    auto wrapper_task = [&]() -> executor_void_task {
//        // Steal should forward executor tasks unchanged
//        co_await exec.steal(exec_task());
//        latch.count_down();
//        co_return;
//    };
//
//    sched.post([t = wrapper_task()]() mutable {});
//
//    latch.wait();
//    EXPECT_TRUE(executed.load());
//    sched.await_stop();
//}
//
//// ========== Multiple Executors Tests ==========
//
//CORO_ITEST(executor, multiple_executors_same_scheduler_work_together) {
//    scheduler sched(autostart, 2);
//    executor exec1(sched);
//    executor exec2(sched);
//    std::atomic<int> counter{0};
//    std::latch latch(2);
//
//    auto task1 = [&]() -> executor_void_task {
//        co_await exec1.schedule();
//        counter.fetch_add(1, std::memory_order_relaxed);
//        latch.count_down();
//        co_return;
//    };
//
//    auto task2 = [&]() -> executor_void_task {
//        co_await exec2.schedule();
//        counter.fetch_add(1, std::memory_order_relaxed);
//        latch.count_down();
//        co_return;
//    };
//
//    sched.post([t = task1()]() mutable {});
//    sched.post([t = task2()]() mutable {});
//
//    latch.wait();
//    EXPECT_EQ(counter.load(), 2);
//    sched.await_stop();
//}
//
//CORO_ITEST(executor, executor_with_different_schedulers) {
//    scheduler sched1(autostart, 1);
//    scheduler sched2(autostart, 1);
//    executor exec1(sched1);
//    executor exec2(sched2);
//    std::atomic<int> counter{0};
//    std::latch latch(2);
//
//    auto task1 = [&]() -> executor_void_task {
//        co_await exec1.schedule();
//        counter.fetch_add(1, std::memory_order_relaxed);
//        latch.count_down();
//        co_return;
//    };
//
//    auto task2 = [&]() -> executor_void_task {
//        co_await exec2.schedule();
//        counter.fetch_add(1, std::memory_order_relaxed);
//        latch.count_down();
//        co_return;
//    };
//
//    sched1.post([t = task1()]() mutable {});
//    sched2.post([t = task2()]() mutable {});
//
//    latch.wait();
//    EXPECT_EQ(counter.load(), 2);
//    
//    sched1.await_stop();
//    sched2.await_stop();
//}
//
//// ========== Complex Task Chains ==========
//
//CORO_ITEST(executor, chained_scheduling) {
//    scheduler sched(autostart, 1);
//    executor exec(sched);
//    std::atomic<int> counter{0};
//    std::latch latch(1);
//
//    auto task = [&]() -> executor_void_task {
//        counter.fetch_add(1, std::memory_order_relaxed);
//        co_await exec.schedule();
//        counter.fetch_add(1, std::memory_order_relaxed);
//        co_await exec.schedule();
//        counter.fetch_add(1, std::memory_order_relaxed);
//        latch.count_down();
//        co_return;
//    };
//
//    sched.post([t = task()]() mutable {});
//
//    latch.wait();
//    EXPECT_EQ(counter.load(), 3);
//    sched.await_stop();
//}
//
//CORO_ITEST(executor, nested_spawn_and_steal) {
//    scheduler sched(autostart, 2);
//    executor exec(sched);
//    std::atomic<int> counter{0};
//    std::latch latch(1);
//
//    auto inner_task = [&]() -> executor_task<int> {
//        counter.fetch_add(1, std::memory_order_relaxed);
//        co_return 42;
//    };
//
//    auto middle_task = [&, exec]() -> executor_task<int> {
//        int value = co_await exec.steal(inner_task());
//        counter.fetch_add(value, std::memory_order_relaxed);
//        co_return value;
//    };
//
//    auto outer_task = [&]() -> executor_void_task {
//        co_await exec.spawn(middle_task());
//        counter.fetch_add(1, std::memory_order_relaxed);
//        latch.count_down();
//        co_return;
//    };
//
//    sched.post([t = outer_task()]() mutable {});
//
//    latch.wait();
//    EXPECT_EQ(counter.load(), 44); // 1 + 42 + 1
//    sched.await_stop();
//}
//
//} // namespace cth::co