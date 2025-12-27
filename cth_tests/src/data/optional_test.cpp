#include "cth/data/optional.hpp"

#include "test.hpp"

#include <string>
#include <memory>
#include <vector>
#include <optional>

// Assuming the optional header is included via a precompiled header or similar mechanism
// as per the example structure.

namespace cth::dt {

DATA_TEST(optional, basic_construction_and_access) {
    // Default / Nullopt
    optional<int> empty;
    EXPECT_FALSE(empty.has_value());
    EXPECT_FALSE(static_cast<bool>(empty));

    optional<int> explicitEmpty(std::nullopt);
    EXPECT_FALSE(explicitEmpty.has_value());

    // Value construction
    optional<int> val(42);
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, 42);
    EXPECT_EQ(val.value(), 42);

    // In place
    optional<std::string> s(std::in_place, 3, 'a');
    EXPECT_TRUE(s.has_value());
    EXPECT_EQ(*s, "aaa");

    // Value or
    EXPECT_EQ(empty.value_or(100), 100);
    EXPECT_EQ(val.value_or(100), 42);
}

DATA_TEST(optional, modification_and_swap) {
    optional<int> a(10);
    optional<int> b(20);

    a.swap(b);
    EXPECT_EQ(*a, 20);
    EXPECT_EQ(*b, 10);

    a.reset();
    EXPECT_FALSE(a.has_value());

    a.emplace(50);
    EXPECT_TRUE(a.has_value());
    EXPECT_EQ(*a, 50);
}

DATA_TEST(optional, reference_specialization) {
    int x = 10;
    int y = 20;

    optional<int&> ref_opt(x);
    EXPECT_TRUE(ref_opt.has_value());
    EXPECT_EQ(*ref_opt, 10);

    // Modify underlying
    x = 15;
    EXPECT_EQ(*ref_opt, 15);

    // Modify via optional
    *ref_opt = 30;
    EXPECT_EQ(x, 30);

    // Rebind
    ref_opt = y;
    EXPECT_EQ(*ref_opt, 20);

    // Ensure x was not modified by the rebind
    EXPECT_EQ(x, 30);

    // Nullopt assignment
    ref_opt = std::nullopt;
    EXPECT_FALSE(ref_opt.has_value());

    // Value or (returns T, not T&)
    int a = 99;
    EXPECT_EQ(ref_opt.value_or(a), 99);

    ref_opt = x;
    EXPECT_EQ(ref_opt.value_or(a), 30);
}

DATA_TEST(optional, void_specialization) {
    optional<void> v;
    EXPECT_FALSE(v.has_value());

    optional<void> v2(std::in_place);
    EXPECT_TRUE(v2.has_value());

    v.emplace();
    EXPECT_TRUE(v.has_value());

    v.reset();
    EXPECT_FALSE(v.has_value());

    // value() check
    EXPECT_NO_THROW(v2.value());
    EXPECT_THROW(v.value(), std::bad_optional_access);
}

DATA_TEST(optional, monadic_transform) {
    // 1. Value -> Value
    optional<int> i(5);
    auto r1 = i.transform([](int n) { return n * 2; });
    EXPECT_TRUE(r1.has_value());
    EXPECT_EQ(*r1, 10);

    // 2. Value -> Different Type
    auto r2 = i.transform([](int n) { return std::to_string(n); });
    EXPECT_EQ(*r2, "5");

    // 3. Empty -> Empty
    optional<int> empty;
    auto r3 = empty.transform([](int n) { return n * 2; });
    EXPECT_FALSE(r3.has_value());

    // 4. Value -> Void
    bool sideEffect = false;
    auto r4 = i.transform([&](int) { sideEffect = true; });
    EXPECT_TRUE(sideEffect);
    EXPECT_TRUE(r4.has_value()); // optional<void> should be engaged

    // 5. Void -> Value
    optional<void> v(std::in_place);
    auto r5 = v.transform([]() { return 42; });
    EXPECT_TRUE(r5.has_value());
    EXPECT_EQ(*r5, 42);
}

DATA_TEST(optional, monadic_and_then) {
    auto safe_div = [](int n) -> optional<int> {
        if(n == 0) return std::nullopt;
        return 100 / n;
    };

    optional<int> a(2);
    optional<int> b(0);
    optional<int> c(std::nullopt);

    // Success chain
    auto r1 = a.and_then(safe_div);
    EXPECT_TRUE(r1.has_value());
    EXPECT_EQ(*r1, 50);

    // Failure in chain
    auto r2 = b.and_then(safe_div);
    EXPECT_FALSE(r2.has_value());

    // Failure at start
    auto r3 = c.and_then(safe_div);
    EXPECT_FALSE(r3.has_value());

    // Void -> Optional
    optional<void> v(std::in_place);
    auto r4 = v.and_then([]() -> optional<std::string> { return "hello"; });
    EXPECT_TRUE(r4.has_value());
    EXPECT_EQ(*r4, "hello");
}

DATA_TEST(optional, monadic_or_else) {
    optional<int> a(10);
    optional<int> empty;

    // Has value: returns self
    auto r1 = a.or_else([] { return optional<int>(20); });
    EXPECT_EQ(*r1, 10);

    // Empty: invokes function
    auto r2 = empty.or_else([] { return optional<int>(20); });
    EXPECT_EQ(*r2, 20);

    // Chaining recovery
    auto r3 = empty
              .or_else([] { return optional<int>(std::nullopt); }) // still fails
              .or_else([] { return optional<int>(30); }); // recovers
    EXPECT_EQ(*r3, 30);
}

DATA_TEST(optional, move_semantics) {
    auto ptr = std::make_unique<int>(123);
    optional<std::unique_ptr<int>> opt(std::move(ptr));

    EXPECT_TRUE(opt.has_value());
    EXPECT_EQ(**opt, 123);
    EXPECT_EQ(ptr, nullptr); // Original moved from

    // Test transform with move-only type
    auto res = std::move(opt).transform(
        [](std::unique_ptr<int> p) {
            return *p + 1;
        }
    );

    EXPECT_EQ(*res, 124);
    // opt is now in a valid but unspecified state (likely empty unique_ptr inside)
}

DATA_TEST(optional, exceptions) {
    optional<int> empty;
    EXPECT_THROW(empty.value(), std::bad_optional_access);

    optional<int&> empty_ref;
    EXPECT_THROW(empty_ref.value(), std::bad_optional_access);

    optional<void> empty_void;
    EXPECT_THROW(empty_void.value(), std::bad_optional_access);
}

}
