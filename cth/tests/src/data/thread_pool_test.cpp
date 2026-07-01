#include "cth/data/thread_pool.hpp"
#include "cth/constants.hpp"
#include "test.hpp"

#include <atomic>
#include <cstddef>
#include <latch>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>


namespace cth::dt {

// ------------------------------
// helpers
// ------------------------------

// manipulator that resets released instances, used to verify the "reset (if supported)" contract
struct ResettableObject {
    int value;
    bool isDirty = false;

    ResettableObject(int v) : value(v) {}

    void touch() { isDirty = true; }
};

struct ObjectResetter {
    void reset(ResettableObject& obj) {
        obj.value = 0;
        obj.isDirty = false;
    }
};

// captures @ref n distinct thread ids belonging to threads that are all alive at the same time,
// guaranteeing the ids are pairwise distinct (an id may otherwise be recycled after a thread ends)
[[nodiscard]] std::vector<std::thread::id> distinctIds(size_t n) {
    std::vector<std::thread::id> ids(n);
    std::latch captured{static_cast<std::ptrdiff_t>(n)};
    std::atomic_bool release{false};

    std::vector<std::jthread> threads;
    threads.reserve(n);
    for(size_t i = 0; i < n; ++i)
        threads.emplace_back([&, i] {
            ids[i] = std::this_thread::get_id();
            captured.count_down();
            release.wait(false);
        });

    captured.wait(); // all ids recorded while every thread is still running
    release = true;
    release.notify_all();

    return ids;
}


// ------------------------------
// construction / accessors
// ------------------------------

DATA_TEST(thread_pool, initial_state_is_empty) {
    thread_pool<int> tp;

    EXPECT_EQ(tp.capacity(), 0);
    EXPECT_EQ(tp.remaining(), 0);
    EXPECT_EQ(tp.bound(), 0);
    EXPECT_TRUE(tp.exhausted());
}

DATA_TEST(thread_pool, emplace_increases_capacity_by_one) {
    thread_pool<int> tp;

    tp.emplace(10);
    EXPECT_EQ(tp.capacity(), 1);
    EXPECT_EQ(tp.remaining(), 1);
    EXPECT_FALSE(tp.exhausted());

    tp.emplace(20);
    EXPECT_EQ(tp.capacity(), 2);
    EXPECT_EQ(tp.remaining(), 2);
}

DATA_TEST(thread_pool, append_range_increases_capacity_by_range_size) {
    thread_pool<int> tp;
    std::vector<int> input = {1, 2, 3, 4};

    tp.append_range(input);

    EXPECT_EQ(tp.capacity(), 4);
    EXPECT_EQ(tp.remaining(), 4);
    EXPECT_EQ(tp.bound(), 0);
}

DATA_TEST(thread_pool, has_reset_reflects_manipulator) {
    // documented static constexpr mirroring the underlying pool
    EXPECT_FALSE((thread_pool<int>::HAS_RESET));
    EXPECT_TRUE((thread_pool<ResettableObject, ObjectResetter>::HAS_RESET));
}


// ------------------------------
// acquire: binds a resource to a thread id
// ------------------------------

DATA_TEST(thread_pool, acquire_binds_resource_to_id) {
    thread_pool<int> tp;
    tp.emplace(100);
    tp.emplace(200);

    // first acquire pulls a fresh resource from the pool
    int& r = tp.acquire();
    EXPECT_EQ(tp.remaining(), 1);
    EXPECT_EQ(tp.bound(), 1);

    bool validVal = (r == 100 || r == 200);
    EXPECT_TRUE(validVal);
}

DATA_TEST(thread_pool, acquire_is_idempotent_for_same_id) {
    // "a thread acquires a resource once and keeps it across calls"
    thread_pool<int> tp;
    tp.emplace(1);
    tp.emplace(2);

    int& first = tp.acquire();
    size_t const remainingAfterFirst = tp.remaining();

    // repeated acquires by the same id return the very same resource without touching the pool
    int& second = tp.acquire();
    int& third = tp.acquire();

    EXPECT_EQ(&first, &second);
    EXPECT_EQ(&first, &third);
    EXPECT_EQ(tp.remaining(), remainingAfterFirst); // no additional resource consumed
    EXPECT_EQ(tp.bound(), 1);                       // still only one owner
}

DATA_TEST(thread_pool, distinct_ids_get_distinct_resources) {
    // "every thread owns at most one resource at a time"
    thread_pool<int> tp;
    tp.emplace(10);
    tp.emplace(20);
    tp.emplace(30);

    auto const ids = distinctIds(3);

    int& a = tp.acquire(ids[0]);
    int& b = tp.acquire(ids[1]);
    int& c = tp.acquire(ids[2]);

    EXPECT_EQ(tp.bound(), 3);
    EXPECT_EQ(tp.remaining(), 0);
    EXPECT_TRUE(tp.exhausted());

    // three owners hold three distinct resources
    std::unordered_set<int*> refs{&a, &b, &c};
    EXPECT_EQ(refs.size(), 3);
}


// ------------------------------
// release: returns a bound resource to the pool
// ------------------------------

DATA_TEST(thread_pool, release_returns_resource_to_pool) {
    thread_pool<int> tp;
    tp.emplace(42);

    int& r = tp.acquire();
    EXPECT_TRUE(tp.exhausted());
    EXPECT_EQ(tp.bound(), 1);

    tp.release();
    EXPECT_FALSE(tp.exhausted());
    EXPECT_EQ(tp.remaining(), 1);
    EXPECT_EQ(tp.bound(), 0);

    // resource is available again after release
    int& again = tp.acquire();
    EXPECT_EQ(&r, &again);
}

DATA_TEST(thread_pool, release_resets_resource_when_supported) {
    // "the resource is reset (if supported) and can be acquired again"
    thread_pool<ResettableObject, ObjectResetter> tp;
    tp.emplace(7);

    ResettableObject& obj = tp.acquire();
    obj.value = 99;
    obj.touch();

    tp.release();

    ResettableObject& reacquired = tp.acquire();
    EXPECT_EQ(&obj, &reacquired);
    EXPECT_EQ(reacquired.value, 0);
    EXPECT_FALSE(reacquired.isDirty);
}

DATA_TEST(thread_pool, release_targets_the_given_id) {
    thread_pool<int> tp;
    tp.emplace(1);
    tp.emplace(2);

    auto const ids = distinctIds(2);

    [[maybe_unused]] auto _ = tp.acquire(ids[0]);
    [[maybe_unused]] auto __ = tp.acquire(ids[1]);
    EXPECT_EQ(tp.bound(), 2);

    // releasing one id leaves the other's binding intact
    tp.release(ids[0]);
    EXPECT_EQ(tp.bound(), 1);
    EXPECT_EQ(tp.remaining(), 1);
}

 
// ------------------------------
// clear
// ------------------------------

DATA_TEST(thread_pool, clear_releases_every_binding) {
    thread_pool<int> tp;
    tp.emplace(1);
    tp.emplace(2);
    tp.emplace(3);

    auto const ids = distinctIds(3); 
    [[maybe_unused]] auto _ = tp.acquire(ids[0]);
    [[maybe_unused]] auto __ = tp.acquire(ids[1]);
    [[maybe_unused]] auto ___ = tp.acquire(ids[2]);
    EXPECT_TRUE(tp.exhausted());
    EXPECT_EQ(tp.bound(), 3);

    tp.clear();

    EXPECT_EQ(tp.bound(), 0);
    EXPECT_EQ(tp.remaining(), tp.capacity());
    EXPECT_FALSE(tp.exhausted());
}

DATA_TEST(thread_pool, clear_resets_active_objects_when_supported) {
    thread_pool<ResettableObject, ObjectResetter> tp;
    tp.emplace(0);
    tp.emplace(0);

    ResettableObject& a = tp.acquire(distinctIds(1)[0]);
    a.value = 123;
    a.touch();

    tp.clear();

    ResettableObject& x = tp.acquire();
    ResettableObject& y = tp.acquire();
    EXPECT_EQ(x.value, 0);
    EXPECT_EQ(y.value, 0);
    EXPECT_FALSE(x.isDirty);
    EXPECT_FALSE(y.isDirty);
}


// ------------------------------
// thread safety
// ------------------------------

DATA_TEST(thread_pool, concurrent_acquire_gives_each_thread_a_distinct_resource) {
    // exercises the "thread safe" claim: many threads acquire simultaneously
    constexpr size_t THREADS = 16;

    thread_pool<int> tp;
    for(size_t i = 0; i < THREADS; ++i)
        tp.emplace(static_cast<int>(i));

    std::vector<int*> acquired(THREADS, nullptr);
    std::latch start{THREADS};

    {
        std::vector<std::jthread> workers;
        workers.reserve(THREADS);
        for(size_t i = 0; i < THREADS; ++i)
            workers.emplace_back([&, i] {
                start.arrive_and_wait(); // maximise contention
                acquired[i] = &tp.acquire();
            });
    } // join all

    EXPECT_EQ(tp.bound(), THREADS);
    EXPECT_TRUE(tp.exhausted());

    // every thread received a distinct resource, none null
    std::unordered_set<int*> unique{acquired.begin(), acquired.end()};
    EXPECT_EQ(unique.size(), THREADS);
    EXPECT_FALSE(unique.contains(nullptr));
}

DATA_TEST(thread_pool, concurrent_acquire_release_cycles_stay_consistent) {
    constexpr size_t THREADS = 8;
    constexpr size_t ITERATIONS = 200;

    thread_pool<int> tp;
    for(size_t i = 0; i < THREADS; ++i)
        tp.emplace(0);

    {
        std::vector<std::jthread> workers;
        workers.reserve(THREADS);
        for(size_t i = 0; i < THREADS; ++i)
            workers.emplace_back([&] {
                for(size_t it = 0; it < ITERATIONS; ++it) {
                    int& r = tp.acquire();
                    r += 1;
                    tp.release();
                }
            });
    } // join all

    // all bindings released, no resource lost or duplicated
    EXPECT_EQ(tp.bound(), 0);
    EXPECT_EQ(tp.remaining(), tp.capacity());
    EXPECT_EQ(tp.capacity(), THREADS);
}


// ------------------------------
// non-trivial value type
// ------------------------------

DATA_TEST(thread_pool, works_with_non_trivial_value_type) {
    thread_pool<std::string> tp;
    tp.emplace("hello");

    std::string& s = tp.acquire();
    EXPECT_EQ(s, "hello");
    s = "world";

    tp.release();

    // default manipulator does not reset -> state persists
    std::string& s2 = tp.acquire();
    EXPECT_EQ(s2, "world");
    EXPECT_EQ(&s, &s2);
}

}
