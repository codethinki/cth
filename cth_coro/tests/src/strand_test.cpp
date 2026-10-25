#include "test.hpp"

#include <cth/coro/atomic_fence.hpp>
#include <cth/coro/executor.hpp>
#include <cth/coro/scheduler.hpp>
#include <cth/coro/strand.hpp>
#include <cth/coro/tasks/executor_task.hpp>
#include <cth/coro/this_coro.hpp>
#include <cth/os/fence.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <latch>
#include <ranges>
#include <semaphore>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


namespace cth::co {

namespace {
    constexpr auto TIMEOUT = std::chrono::seconds{5};
}

CORO_TEST(strand, jobs_run_one_at_a_time_in_post_order) {
    constexpr size_t workers = 4;
    constexpr size_t jobs = 1000;

    std::atomic<size_t> running{0};
    std::atomic<size_t> maxRunning{0};
    std::vector<size_t> order{};
    order.reserve(jobs);
    std::latch done{jobs};

    scheduler sched{autostart, workers};
    strand serial{sched};

    for(size_t i = 0; i < jobs; ++i)
        serial.post([&, i] {
            auto const now = running.fetch_add(1) + 1;
            auto max = maxRunning.load();
            while(max < now && !maxRunning.compare_exchange_weak(max, now)) {}

            std::this_thread::yield(); // widens the window for an overlap
            order.push_back(i); // the strand serializes the access

            running.fetch_sub(1);
            done.count_down();
        });

    done.wait();

    EXPECT_FALSE(serial.owns_thread());
    EXPECT_EQ(maxRunning.load(), 1);
    EXPECT_TRUE(std::ranges::equal(order, std::views::iota(size_t{0}, jobs)));
}

CORO_TEST(strand, await_handle_calls_inside_strand) {
    os::fence fence{};
    std::atomic<bool> inStrand{false};
    atomic_fence done{};

    scheduler sched{autostart, 4};
    strand serial{sched};

    serial.await(
        fence.native_handle(),
        [&] {
            inStrand = serial.owns_thread();
            done.signal();
        }
    );
    fence.signal();

    done.wait();
    EXPECT_TRUE(inStrand);
}

CORO_TEST(strand, await_time_point_calls_inside_strand) {
    std::atomic<bool> inStrand{false};
    atomic_fence done{};

    scheduler sched{autostart, 4};
    strand serial{sched};

    serial.await(
        chrono::clock_t::now() + std::chrono::milliseconds{1},
        [&] {
            inStrand = serial.owns_thread();
            done.signal();
        }
    );

    done.wait();
    EXPECT_TRUE(inStrand);
}

CORO_TEST(strand, schedule_continues_inside_strand) {
    scheduler sched{autostart, 4};
    strand serial{sched};

    auto task = [](strand const& target) -> executor_task<std::pair<bool, bool>> {
        bool const before = target.owns_thread();
        co_await executor{target}.schedule();
        co_return std::pair{before, target.owns_thread()};
    };

    auto const [before, after] = executor{sched}.sync_wait(task(serial));
    EXPECT_FALSE(before);
    EXPECT_TRUE(after);
}

CORO_TEST(strand, spawned_task_waits_inside_strand) {
    scheduler sched{autostart, 4};
    strand serial{sched};

    auto task = []() -> executor_task<std::pair<context const*, bool>> {
        co_await this_coro::wait(std::chrono::microseconds{200});

        context const& current = co_await this_coro::context;
        co_return std::pair{&current, current.owns_thread()};
    };

    auto const [current, inStrand] = executor{serial}.sync_wait(task());
    EXPECT_EQ(current, &serial);
    EXPECT_TRUE(inStrand);
}

CORO_TEST(strand, schedule_onto_scheduler_leaves_strand) {
    std::binary_semaphore nextJobRan{0};

    scheduler sched{autostart, 2};
    strand serial{sched};

    auto task = [](strand const& s, executor target, std::binary_semaphore& jobRan)
        -> executor_task<std::tuple<bool, bool, context const*>> {
        s.post([&jobRan] { jobRan.release(); }); // queued behind the current job

        co_await target.schedule();

        bool const ranBeside = jobRan.try_acquire_for(TIMEOUT);
        context const& current = co_await this_coro::context;
        co_return std::tuple{ranBeside, s.owns_thread(), &current};
    };

    auto const [ranBeside, inStrand, current] = executor{serial}.sync_wait(task(serial, sched, nextJobRan));
    EXPECT_TRUE(ranBeside);
    EXPECT_FALSE(inStrand);
    EXPECT_EQ(current, &sched);
}

CORO_TEST(strand, spawn_onto_scheduler_leaves_strand) {
    std::binary_semaphore nextJobRan{0};

    scheduler sched{autostart, 2};
    strand serial{sched};

    auto child = [](strand const& s, std::binary_semaphore& jobRan) -> executor_task<std::pair<bool, bool>> {
        bool const ranBeside = jobRan.try_acquire_for(TIMEOUT);
        co_return std::pair{ranBeside, s.owns_thread()};
    };

    auto task = [](strand const& s, executor target, std::binary_semaphore& jobRan, auto c)
        -> executor_task<std::pair<bool, bool>> {
        s.post([&jobRan] { jobRan.release(); }); // queued behind the current job

        co_return co_await target.spawn(c(s, jobRan));
    };

    auto const [ranBeside, inStrand] = executor{serial}.sync_wait(task(serial, sched, nextJobRan, child));
    EXPECT_TRUE(ranBeside);
    EXPECT_FALSE(inStrand);
}

CORO_TEST(strand, current_is_the_context_of_the_job) {
    std::atomic<context const*> inStrand{nullptr};
    std::atomic<context const*> inScheduler{nullptr};
    std::latch done{2};

    scheduler sched{autostart, 2};
    strand serial{sched};

    serial.post([&] {
        inStrand = current_context();
        done.count_down();
    });
    sched.post([&] {
        inScheduler = current_context();
        done.count_down();
    });
    done.wait();

    EXPECT_EQ(current_context(), nullptr);
    EXPECT_EQ(inStrand.load(), &serial);
    EXPECT_EQ(inScheduler.load(), &sched);
}

CORO_TEST(strand, caller_continues_outside_the_strand_its_child_ended_in) {
    scheduler sched{autostart, 2};
    strand serial{sched};

    auto child = [](executor end) -> executor_task<void> { co_await end.schedule(); };

    auto task = [](scheduler const& b, strand const& s, auto c)
        -> executor_task<std::tuple<bool, bool, context const*>> {
        co_await executor{b}.spawn(c(executor{s}));
        context const& current = co_await this_coro::context;
        co_return std::tuple{s.owns_thread(), b.owns_thread(), &current};
    };

    auto const [inStrand, onScheduler, current] = executor{sched}.sync_wait(task(sched, serial, child));
    EXPECT_FALSE(inStrand);
    EXPECT_TRUE(onScheduler);
    EXPECT_EQ(current, &sched);
}

CORO_TEST(strand, caller_in_strand_continues_in_strand_after_spawn) {
    scheduler sched{autostart, 2};
    strand serial{sched};

    auto child = [](scheduler const& b, strand const& s) -> executor_task<std::pair<bool, bool>> {
        co_return std::pair{b.owns_thread(), s.owns_thread()};
    };

    auto task = [](scheduler const& b, strand const& s, auto c)
        -> executor_task<std::tuple<bool, bool, bool, context const*>> {
        auto const [childOnScheduler, childInStrand] = co_await executor{b}.spawn(c(b, s));
        context const& current = co_await this_coro::context;
        co_return std::tuple{childOnScheduler, childInStrand, s.owns_thread(), &current};
    };

    auto const [childOnScheduler, childInStrand, inStrand, current] =
        executor{serial}.sync_wait(task(sched, serial, child));
    EXPECT_TRUE(childOnScheduler);
    EXPECT_FALSE(childInStrand);
    EXPECT_TRUE(inStrand);
    EXPECT_EQ(current, &serial);
}

CORO_TEST(strand, scheduler_jobs_run_beside_strand_job) {
    std::binary_semaphore strandRunning{0};
    std::binary_semaphore schedulerRan{0};
    std::binary_semaphore strandDone{0};
    std::atomic<bool> overlapped{false};

    scheduler sched{autostart, 2};
    strand serial{sched};

    serial.post([&] {
        strandRunning.release();
        overlapped = schedulerRan.try_acquire_for(TIMEOUT);
        strandDone.release();
    });

    ASSERT_TRUE(strandRunning.try_acquire_for(TIMEOUT));
    sched.post([&] { schedulerRan.release(); });

    ASSERT_TRUE(strandDone.try_acquire_for(2 * TIMEOUT));
    EXPECT_TRUE(overlapped);
}

CORO_TEST(strand, move_only) {
    EXPECT_FALSE(std::is_copy_constructible_v<strand>);
    EXPECT_FALSE(std::is_copy_assignable_v<strand>);
    EXPECT_TRUE(std::is_nothrow_move_constructible_v<strand>);
    EXPECT_TRUE(std::is_nothrow_move_assignable_v<strand>);
}

}
