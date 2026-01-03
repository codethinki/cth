#include "cth/data/pool.hpp"
#include "test.hpp"

#include <string>
#include <vector>
#include <memory>
#include <algorithm>


namespace cth::dt {

// helper for manipulator mechanics
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

DATA_TEST(pool, basic_capacity_and_emplace) {
    pool<int> p;

    // initial state
    EXPECT_EQ(p.capacity(), 0);
    EXPECT_EQ(p.remaining(), 0);
    EXPECT_TRUE(p.exhausted());

    // emplace increases capacity/remaining
    p.emplace(10);
    EXPECT_EQ(p.capacity(), 1);
    EXPECT_EQ(p.remaining(), 1);
    EXPECT_FALSE(p.exhausted());

    p.emplace(20);
    EXPECT_EQ(p.capacity(), 2);
    EXPECT_EQ(p.remaining(), 2);
}

DATA_TEST(pool, acquire_behavior) {
    pool<int> p;
    p.emplace(100);
    p.emplace(200);

    size_t initialCap = p.capacity();

    // acquire first
    int& r1 = p.acquire();
    EXPECT_EQ(p.remaining(), initialCap - 1);
    EXPECT_FALSE(p.exhausted());

    // verify value validity
    bool isValidVal = (r1 == 100 || r1 == 200);
    EXPECT_TRUE(isValidVal);

    // acquire second
    int& r2 = p.acquire();
    EXPECT_EQ(p.remaining(), initialCap - 2);
    EXPECT_TRUE(p.exhausted());

    // ensure distinct objects
    EXPECT_NE(&r1, &r2);
}

DATA_TEST(pool, release_and_reuse) {
    pool<std::string> p;
    p.emplace("Hello");

    // acquire
    std::string& s = p.acquire();
    EXPECT_EQ(s, "Hello");
    EXPECT_TRUE(p.exhausted());

    // modify
    s = "World";

    // release
    p.release(s);
    EXPECT_FALSE(p.exhausted());
    EXPECT_EQ(p.remaining(), 1);

    // re-acquire
    std::string& s2 = p.acquire();

    // contract: release allows re-acquire
    // default manipulator: no reset, state persists
    EXPECT_EQ(s2, "World");
    EXPECT_EQ(&s, &s2);
}

DATA_TEST(pool, append_range) {
    pool<int> p{};
    std::vector<int> input = {1, 2, 3};

    p.append_range(input);

    EXPECT_EQ(p.capacity(), 3);
    EXPECT_EQ(p.remaining(), 3);

    // verify acquire all
    std::vector<int> acquiredVals;
    for(int i = 0; i < 3; ++i) {
        acquiredVals.push_back(p.acquire());
    }

    EXPECT_TRUE(p.exhausted());

    // verify contents match input
    for(int val : input) {
        bool found = false;
        for(int acq : acquiredVals) {
            if(acq == val) found = true;
        }
        EXPECT_TRUE(found);
    }
}

DATA_TEST(pool, manipulator_reset_on_release) {
    pool<ResettableObject, ObjectResetter> p;
    p.emplace(42);

    // acquire and modify
    ResettableObject& obj = p.acquire();
    EXPECT_EQ(obj.value, 42);
    EXPECT_FALSE(obj.isDirty);

    obj.value = 99;
    obj.touch();

    // release triggers ObjectResetter::reset
    p.release(obj);

    // re-acquire
    ResettableObject& obj2 = p.acquire();

    // verify reset
    EXPECT_EQ(obj2.value, 0);
    EXPECT_FALSE(obj2.isDirty);
}

DATA_TEST(pool, clear_resets_all) {
    pool<ResettableObject, ObjectResetter> p;
    p.emplace(10);
    p.emplace(20);

    // acquire both
    ResettableObject& r1 = p.acquire();
    ResettableObject& r2 = p.acquire();

    // modify both
    r1.value = 111;
    r2.value = 222;

    EXPECT_TRUE(p.exhausted());

    // clear: resets active objects, makes available
    p.clear();

    EXPECT_EQ(p.remaining(), 2);
    EXPECT_FALSE(p.exhausted());

    // inspect internal state
    ResettableObject& newR1 = p.acquire();
    ResettableObject& newR2 = p.acquire();

    EXPECT_EQ(newR1.value, 0);
    EXPECT_EQ(newR2.value, 0);
}

DATA_TEST(pool, clear_mixed_state) {
    // test clear with mixed acquired/pooled state
    pool<ResettableObject, ObjectResetter> p;
    p.emplace(0);
    p.emplace(0);

    auto& r1 = p.acquire();
    r1.value = 999; // dirty

    // r2 stays in pool

    p.clear();

    EXPECT_EQ(p.remaining(), 2);

    // check previously acquired object reset
    auto& a = p.acquire();
    auto& b = p.acquire();

    EXPECT_EQ(a.value, 0);
    EXPECT_EQ(b.value, 0);
}

DATA_TEST(pool, move_only_types) {
    // ensure perfect forwarding and unique_ptr support
    pool<std::unique_ptr<int>> p;

    p.emplace(std::make_unique<int>(1337));

    std::unique_ptr<int>& ptrRef = p.acquire();
    EXPECT_TRUE(ptrRef != nullptr);
    EXPECT_EQ(*ptrRef, 1337);

    // move out allowed, must restore before release
    std::unique_ptr<int> stolen = std::move(ptrRef);
    EXPECT_EQ(*stolen, 1337);
    EXPECT_EQ(ptrRef, nullptr);

    // restore
    ptrRef = std::move(stolen);
    p.release(ptrRef);

    EXPECT_EQ(p.remaining(), 1);
}

}
