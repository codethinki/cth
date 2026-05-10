#include "cth/data/ringalloc.hpp"

#include "test.hpp"

#include <thread>
#include <vector>

namespace cth::dt {

DATA_TEST(ring_alloc, size_and_capacity) {
    constexpr size_t expected = 100;
    ring_alloc alloc{expected};

    ASSERT_EQ(expected, alloc.size());
    ASSERT_EQ(expected, alloc.capacity());
}

DATA_TEST(ring_alloc, initial_state) {
    ring_alloc alloc{100};

    ASSERT_EQ(0, alloc.allocated());
    ASSERT_EQ(100, alloc.max_alloc());
}

DATA_TEST(ring_alloc, basic_allocation) {
    ring_alloc alloc{100};

    auto res = alloc.alloc(20);
    ASSERT_TRUE(res.has_value());
    ASSERT_EQ(0, res->begin);

    ASSERT_EQ(20, alloc.allocated());
    ASSERT_EQ(80, alloc.max_alloc());
}

DATA_TEST(ring_alloc, allocation_fails_when_full) {
    ring_alloc alloc{100};

    auto res1 = alloc.alloc(60);
    ASSERT_TRUE(res1.has_value());

    auto res2 = alloc.alloc(50);
    ASSERT_FALSE(res2.has_value()); // 60 + 50 > 100

    ASSERT_EQ(60, alloc.allocated());
}

DATA_TEST(ring_alloc, wrap_around_padding) {
    ring_alloc alloc{100};

    auto res1 = alloc.alloc(80);
    ASSERT_TRUE(res1.has_value());

    alloc.free(res1->cleanup);

    // At this point:
    // Head is 80, Tail is 80
    // We request 40. 80 + 40 > 100, so it must wrap.
    // It should pad 20, wrap to 0, and allocate 40. 
    // Total consumed this step: 60.
    auto res2 = alloc.alloc(40);
    ASSERT_TRUE(res2.has_value());
    ASSERT_EQ(0, res2->begin); // Successfully wrapped to beginning

    // Allocated includes the 20 bytes of dead padding space + 40 bytes actual
    ASSERT_EQ(60, alloc.allocated());
}

DATA_TEST(ring_alloc, exact_boundary_allocation) {
    ring_alloc alloc{100};

    auto res1 = alloc.alloc(50);
    alloc.free(res1->cleanup);

    // Head is 50. Allocate exactly 50 to hit the exact boundary
    auto res2 = alloc.alloc(50);
    ASSERT_TRUE(res2.has_value());
    ASSERT_EQ(50, res2->begin);

    // Since it hit exactly 100, it shouldn't pad.
    ASSERT_EQ(50, alloc.allocated());

    // The buffer is now "full" logically until wrapped again
    ASSERT_EQ(50, alloc.max_alloc());
}

DATA_TEST(ring_alloc, cumulative_free_behavior) {
    ring_alloc alloc{100};

    auto r1 = alloc.alloc(10);
    auto r2 = alloc.alloc(20);
    auto r3 = alloc.alloc(30);

    ASSERT_EQ(60, alloc.allocated());

    // Cumulative free: freeing the 2nd allocation assumes the 1st is also freed
    alloc.free(r2->cleanup);

    // Head is 60. Tail should jump to 30 (end of r2).
    ASSERT_EQ(30, alloc.allocated());

    // Freeing an older token should safely do nothing (tail stays 30)
    alloc.free(r1->cleanup);
    ASSERT_EQ(30, alloc.allocated());

    // Freeing the newest token releases the rest
    alloc.free(r3->cleanup);
    ASSERT_EQ(0, alloc.allocated());
}

DATA_TEST(ring_alloc, max_alloc_calculations) {
    ring_alloc alloc{100};

    auto r1 = alloc.alloc(70);
    alloc.free(r1->cleanup);

    // State: Head = 70, Tail = 70
    // Naive max = 30 (to end). Wrap max = 70 (from start). Max = 70.
    ASSERT_EQ(70, alloc.max_alloc());

    auto r2 = alloc.alloc(20);
    // State: Head = 90, Tail = 70
    // Naive max = 10. Wrap max = 70. Max = 70.
    ASSERT_EQ(70, alloc.max_alloc());

    auto r3 = alloc.alloc(60);
    // WRAP TRIGGERED!
    // Padding = 10. Actual allocation = 60. Total consumed = 70.
    // State: Head = 160, Tail = 70
    // Physical head = 60. Physical tail = 70.
    // The only free space is between 60 and 70.
    ASSERT_EQ(10, alloc.max_alloc());

    // If we ask for 20, it MUST fail because we only have 10 left.
    auto r4 = alloc.alloc(20);
    ASSERT_FALSE(r4.has_value());

    // But we CAN allocate exactly 10
    auto r5 = alloc.alloc(10);
    ASSERT_TRUE(r5.has_value());

    // Fully exhausted
    ASSERT_EQ(0, alloc.max_alloc());
}

DATA_TEST(ring_alloc, multi_thread_concurrent_allocations) {
    constexpr size_t THREAD_COUNT = 8;
    constexpr size_t ALLOCS_PER_THREAD = 100;
    constexpr size_t ALLOC_SIZE = 10;

    // Capacity exactly large enough to hold all concurrent allocations
    ring_alloc alloc{THREAD_COUNT * ALLOCS_PER_THREAD * ALLOC_SIZE};

    std::vector<std::thread> threads;
    std::atomic<size_t> success_count{0};

    for(size_t i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(
            [&]() {
                for(size_t j = 0; j < ALLOCS_PER_THREAD; ++j) {
                    if(alloc.alloc(ALLOC_SIZE).has_value()) { success_count.fetch_add(1, std::memory_order_relaxed); }
                }
            }
        );
    }

    for(auto& t : threads) { t.join(); }

    // All threads should have successfully allocated without exhausting or overlapping
    ASSERT_EQ(THREAD_COUNT * ALLOCS_PER_THREAD, success_count.load());
    ASSERT_EQ(THREAD_COUNT * ALLOCS_PER_THREAD * ALLOC_SIZE, alloc.allocated());
    ASSERT_EQ(0, alloc.max_alloc());
}

}
