#include "test.hpp"

#include "cth/coro/counter_event.hpp"
#include "cth/coro/executor.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/coro/simple_counting_scope.hpp"
#include "cth/coro/strand.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/utility/concepts.hpp"

#include <cth/coro/atomic_fence.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>
#include <vector>


namespace cth::co {
namespace {
    constexpr auto TIMEOUT = std::chrono::seconds{5};

    // a this_coro caller awaits the awaiter as it is, no capture
    static_assert(this_coro_awaitable<counter_event::awaiter>);

    /**
     * waits for the value, then counts and signals
     */
    executor_task<void> wait_and_signal(
        counter_event const& event,
        uint64_t value,
        std::atomic<size_t>& resumes,
        atomic_fence& done
    ) {
        co_await event.wait(value);
        resumes.fetch_add(1);
        done.signal();
    }

    /**
     * blocks until the jobs posted to the single worker before it ran
     */
    void drain_jobs(scheduler const& sched) {
        atomic_fence ran{};
        sched.post([&ran] { ran.signal(); });
        ran.wait();
    }

    /**
     * blocks until the count reaches the expected count or the timeout passes
     * @return the count reached the expected count
     */
    [[nodiscard]] bool wait_for(std::atomic<size_t> const& count, size_t expected) {
        auto const deadline = std::chrono::steady_clock::now() + TIMEOUT;
        while(count.load() != expected) {
            if(std::chrono::steady_clock::now() > deadline) return false;
            std::this_thread::yield();
        }
        return true;
    }
}

CORO_TEST(counter_event, a_reached_value_does_not_suspend) {
    counter_event const event{2};
    EXPECT_TRUE(event.wait(1).await_ready());
    EXPECT_TRUE(event.wait(2).await_ready());
    EXPECT_FALSE(event.wait(3).await_ready());

    // a suspension would never resume: no set()
    scheduler sched{autostart, 1};
    auto task = [](counter_event const& e) -> executor_task<uint64_t> {
        co_await e.wait(2);
        co_return e.value();
    };
    EXPECT_EQ(executor{sched}.sync_wait(task(event)), 2);
}

CORO_TEST(counter_event, set_resumes_a_wait) {
    // one worker: the task reached its suspension once the jobs before drain_jobs() ran
    scheduler sched{autostart, 1};
    simple_counting_scope tasks{};
    counter_event event{};
    std::atomic<size_t> resumes{0};
    atomic_fence done{};

    executor{sched}.detach(tasks, wait_and_signal(event, 1, resumes, done));
    drain_jobs(sched);
    EXPECT_EQ(resumes.load(), 0);

    event.set(1);
    done.wait();
    tasks.wait();
    EXPECT_EQ(resumes.load(), 1);
    EXPECT_EQ(event.value(), 1);
}

CORO_TEST(counter_event, a_set_resumes_only_the_reached_waits) {
    scheduler sched{autostart, 1};
    simple_counting_scope tasks{};
    counter_event event{};
    std::atomic<size_t> firstResumes{0};
    std::atomic<size_t> secondResumes{0};
    atomic_fence firstDone{};
    atomic_fence secondDone{};

    executor const exec{sched};
    exec.detach(tasks, wait_and_signal(event, 1, firstResumes, firstDone));
    exec.detach(tasks, wait_and_signal(event, 2, secondResumes, secondDone));
    drain_jobs(sched);

    event.set(1);
    firstDone.wait();
    drain_jobs(sched);
    EXPECT_EQ(firstResumes.load(), 1);
    EXPECT_EQ(secondResumes.load(), 0);

    event.set(2);
    secondDone.wait();
    tasks.wait();
    EXPECT_EQ(firstResumes.load(), 1);
    EXPECT_EQ(secondResumes.load(), 1);
}

CORO_TEST(counter_event, a_wait_resumes_on_the_context_of_the_caller) {
    scheduler sched{autostart, 4};
    strand serial{sched};
    simple_counting_scope tasks{};
    counter_event event{};
    std::atomic<bool> onStrand{false};
    atomic_fence suspended{};
    atomic_fence done{};

    auto task = [](counter_event const& e, strand const& s, std::atomic<bool>& result, atomic_fence& f)
        -> executor_task<void> {
        co_await e.wait(1);
        result = s.owns_thread();
        f.signal();
    };
    executor{serial}.detach(tasks, task(event, serial, onStrand, done));
    // behind the job of the task: the wait is pending
    serial.post([&suspended] { suspended.signal(); });
    suspended.wait();

    event.set(1);
    done.wait();
    tasks.wait();
    EXPECT_TRUE(onStrand);
}

CORO_TEST(counter_event, set_posts_the_resume_also_on_the_context_of_the_caller) {
    scheduler sched{autostart, 4};
    strand serial{sched};
    simple_counting_scope tasks{};
    counter_event event{};
    std::atomic<size_t> resumes{0};
    std::atomic<size_t> resumesInSet{1};
    atomic_fence done{};

    executor{serial}.detach(tasks, wait_and_signal(event, 1, resumes, done));

    // in a job of the strand: an inline resume would run before the end of the job
    atomic_fence setDone{};
    serial.post([&] {
        event.set(1);
        resumesInSet = resumes.load();
        setDone.signal();
    });
    setDone.wait();
    done.wait();
    tasks.wait();
    EXPECT_EQ(resumesInSet.load(), 0);
    EXPECT_EQ(resumes.load(), 1);
}

CORO_TEST(counter_event, set_from_another_thread_while_waits_push) {
    constexpr size_t ITERATIONS = 1000;
    constexpr size_t WAITERS = 16;
    constexpr uint64_t VALUES = 64;
    constexpr uint64_t STEP = 4;

    // every waiter waits for each value of its range: first value, then every STEP values
    auto waiter = [](counter_event const& e, uint64_t first, std::atomic<size_t>& count)
        -> executor_task<void> {
        for(uint64_t value = first; value <= VALUES; value += STEP) {
            co_await e.wait(value);
            count.fetch_add(1);
        }
    };

    size_t expected = 0;
    for(size_t i = 0; i < WAITERS; ++i)
        expected += (VALUES - (1 + i % STEP)) / STEP + 1;

    scheduler sched{autostart, 4};
    executor const exec{sched};

    for(size_t i = 0; i < ITERATIONS; ++i) {
        counter_event event{};
        simple_counting_scope tasks{};
        std::atomic<size_t> resumes{0};

        std::thread setter{[&event] {
            for(uint64_t value = 1; value <= VALUES; ++value) {
                event.set(value);
                if(value % 8 == 0) std::this_thread::yield();
            }
        }};
        for(size_t j = 0; j < WAITERS; ++j)
            exec.detach(tasks, waiter(event, 1 + j % STEP, resumes));
        setter.join();

        bool const reached = wait_for(resumes, expected);
        EXPECT_TRUE(reached) << "lost wakeup in iteration " << i << ", got " << resumes.load() << " of "
            << expected;
        // a lost wait resumes with a later value, the tasks end
        if(!reached) event.set(VALUES + 1);

        tasks.wait();
        ASSERT_EQ(resumes.load(), expected) << "iteration " << i;
    }
}

CORO_TEST(counter_event, the_resumed_caller_destroys_the_event_at_once) {
    constexpr size_t ITERATIONS = 5000;

    // owns the event, destroys it at once after the resume: set() and await_suspend() can still run
    auto waiter = [](std::unique_ptr<counter_event> event) -> executor_task<void> {
        co_await event->wait(1);
        event.reset();
    };

    scheduler sched{autostart, 4};
    executor const exec{sched};

    for(size_t i = 0; i < ITERATIONS; ++i) {
        auto event = std::make_unique<counter_event>();
        auto* const eventPtr = event.get();
        simple_counting_scope tasks{};
        atomic_fence setDone{};

        exec.detach(tasks, waiter(std::move(event)));
        // on another worker: the set() races the push of the wait
        sched.post([eventPtr, &setDone] {
            eventPtr->set(1);
            setDone.signal();
        });

        setDone.wait();
        tasks.wait();
    }
}

CORO_TEST(counter_event, moves_copy_the_value) {
    counter_event event{3};

    counter_event moved{std::move(event)};
    EXPECT_EQ(moved.value(), 3);

    counter_event other{};
    other = std::move(moved);
    EXPECT_EQ(other.value(), 3);

    other.set(4);
    EXPECT_TRUE(other.wait(4).await_ready());
}
}
