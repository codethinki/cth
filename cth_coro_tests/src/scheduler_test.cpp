#include "test.hpp"

#include <cth/coro/scheduler.hpp>
#include <cth/coro/executor.hpp>

#include <atomic>

namespace cth::co {

// Helper coroutines
executor_void_task simple_task() {
    co_return;
}

executor_void_task counting_task(std::atomic<int>& counter) {
    counter++;
    co_return;
}

executor_task<int> value_task(int value) {
    co_return value;
}

// ========== Construction Tests ==========

CORO_TEST(scheduler, construct_default) {
    scheduler sched;
    EXPECT_EQ(sched.workers(), 1);
    EXPECT_EQ(sched.active_workers(), 0);
    EXPECT_FALSE(sched.active());
}

CORO_TEST(scheduler, construct_with_workers) {
    scheduler sched(4);
    EXPECT_EQ(sched.workers(), 4);
    EXPECT_EQ(sched.active_workers(), 0);
    EXPECT_FALSE(sched.active());
}

CORO_TEST(scheduler, construct_with_single_worker) {
    scheduler sched(1);
    EXPECT_EQ(sched.workers(), 1);
}

CORO_TEST(scheduler, construct_with_many_workers) {
    scheduler sched(16);
    EXPECT_EQ(sched.workers(), 16);
}

// ========== Method Instantiation Tests ==========

CORO_TEST(scheduler, post_method_compiles) {
    scheduler sched;
    // Just verify it compiles - don't start the scheduler
    // The task won't execute but the method is instantiated
    sched.post(simple_task());
    SUCCEED(); // If we get here, post() compiled
}

CORO_TEST(scheduler, start_method_compiles) {
    scheduler sched;
    sched.start();
    // Immediately stop to keep test fast
    sched.request_stop();
    sched.await_stop();
    SUCCEED();
}

CORO_TEST(scheduler, request_stop_method_compiles) {
    scheduler sched;
    sched.start();
    sched.request_stop(); // Instantiate method
    sched.await_stop();
    SUCCEED();
}

CORO_TEST(scheduler, await_stop_method_compiles) {
    scheduler sched;
    sched.start();
    sched.request_stop();
    sched.await_stop(); // Instantiate method
    SUCCEED();
}

CORO_TEST(scheduler, transfer_exec_method_compiles) {
    scheduler sched;
    auto awaiter = sched.transfer_exec(); // Instantiate method
    EXPECT_FALSE(awaiter.await_ready()); // Verify awaiter interface
    SUCCEED();
}

// ========== Worker Count Tests ==========

CORO_TEST(scheduler, workers_accessor) {
    {
        scheduler sched(1);
        EXPECT_EQ(sched.workers(), 1);
    }
    {
        scheduler sched(2);
        EXPECT_EQ(sched.workers(), 2);
    }
    {
        scheduler sched(4);
        EXPECT_EQ(sched.workers(), 4);
    }
    {
        scheduler sched(8);
        EXPECT_EQ(sched.workers(), 8);
    }
}

CORO_TEST(scheduler, active_workers_accessor_before_start) {
    scheduler sched(4);
    EXPECT_EQ(sched.active_workers(), 0u);
}

// ========== State Tests ==========

CORO_TEST(scheduler, active_before_start) {
    scheduler sched(2);
    EXPECT_FALSE(sched.active());
}

CORO_TEST(scheduler, active_after_start) {
    scheduler sched(1);
    sched.start();
    EXPECT_TRUE(sched.active());
    sched.request_stop();
    sched.await_stop();
}

CORO_TEST(scheduler, active_after_stop) {
    scheduler sched(1);
    sched.start();
    sched.request_stop();
    EXPECT_FALSE(sched.active());
    sched.await_stop();
}

// ========== Move Semantics Tests ==========

CORO_TEST(scheduler, move_construct) {
    scheduler sched1(2);
    EXPECT_EQ(sched1.workers(), 2);
    
    scheduler sched2(std::move(sched1));
    EXPECT_EQ(sched2.workers(), 2);
}

CORO_TEST(scheduler, move_assign) {
    scheduler sched1(3);
    scheduler sched2(1);
    
    sched2 = std::move(sched1);
    EXPECT_EQ(sched2.workers(), 3);
}

// ========== Awaiter Interface Tests ==========

CORO_TEST(scheduler, ctx_switch_awaiter_interface) {
    scheduler sched;
    auto awaiter = sched.transfer_exec();
    
    // Verify awaiter interface
    EXPECT_FALSE(awaiter.await_ready());
    // Don't call await_suspend as it would post to io_context
    // Just verify the method exists and compiles
    SUCCEED();
}

CORO_TEST(scheduler, ctx_switch_awaiter_has_scheduler_ptr) {
    scheduler sched;
    auto awaiter = sched.transfer_exec();
    
    // Verify the awaiter holds a scheduler pointer
    EXPECT_NE(awaiter.sched, nullptr);
}

// ========== Destructor Tests ==========

CORO_TEST(scheduler, destructor_without_start) {
    {
        scheduler sched(2);
        // Destructor should handle not-started state
    }
    SUCCEED();
}

CORO_TEST(scheduler, destructor_after_start_and_stop) {
    {
        scheduler sched(2);
        sched.start();
        sched.request_stop();
        sched.await_stop();
        // Destructor should handle stopped state
    }
    SUCCEED();
}

// ========== Edge Cases ==========

CORO_TEST(scheduler, multiple_posts_without_start) {
    scheduler sched;
    
    // Post multiple tasks before starting
    sched.post(simple_task());
    sched.post(simple_task());
    sched.post(simple_task());
    
    // All should compile and queue (but not execute)
    SUCCEED();
}

CORO_TEST(scheduler, post_different_task_types) {
    scheduler sched;
    
    std::atomic<int> counter{0};
    
    // Verify different task types can be posted
    sched.post(simple_task());
    sched.post(counting_task(counter));
    
    SUCCEED();
}

CORO_TEST(scheduler, start_stop_cycle_without_tasks) {
    scheduler sched(1);
    
    sched.start();
    EXPECT_TRUE(sched.active());
    
    sched.request_stop();
    sched.await_stop();
    EXPECT_FALSE(sched.active());
}

// ========== Consistency Tests ==========

CORO_TEST(scheduler, workers_count_unchanged_after_start) {
    scheduler sched(3);
    size_t workers_before = sched.workers();
    
    sched.start();
    EXPECT_EQ(sched.workers(), workers_before);
    
    sched.request_stop();
    sched.await_stop();
    EXPECT_EQ(sched.workers(), workers_before);
}

CORO_TEST(scheduler, active_workers_zero_before_start) {
    scheduler sched(5);
    EXPECT_EQ(sched.active_workers(), 0u);
}

CORO_TEST(scheduler, can_query_state_multiple_times) {
    scheduler sched(2);
    
    EXPECT_FALSE(sched.active());
    EXPECT_FALSE(sched.active());
    EXPECT_EQ(sched.workers(), 2);
    EXPECT_EQ(sched.workers(), 2);
    EXPECT_EQ(sched.active_workers(), 0u);
    EXPECT_EQ(sched.active_workers(), 0u);
}

} // namespace cth::co