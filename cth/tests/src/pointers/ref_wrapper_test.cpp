#include "cth/test.hpp"

#include "cth/ptr/ref_wrapper.hpp"

#include <concepts>
#include <type_traits>


namespace cth {
namespace {
    constexpr int EXAMPLE_NUMBER = 42;

    struct Base {
        int value;
    };

    struct Derived : Base {};

    /**
     * converts to the base it holds
     */
    struct Holder {
        Base base;

        constexpr operator Base const&() const { return base; }
    };

    constexpr Base CONSTANT{EXAMPLE_NUMBER};
}

CTH_TEST(ref_wrapper, constructs_from_a_reference) {
    Base base{EXAMPLE_NUMBER};

    ref_wrapper<Base> const ref{base};

    EXPECT_EQ(&ref.get(), &base);
}

CTH_TEST(ref_wrapper, constructs_from_a_derived_type) {
    Derived derived{{EXAMPLE_NUMBER}};

    ref_wrapper<Base const> const ref = derived;

    EXPECT_EQ(&ref.get(), &derived);
}

CTH_TEST(ref_wrapper, constructs_from_a_type_with_a_conversion_operator) {
    Holder const holder{{EXAMPLE_NUMBER}};

    ref_wrapper<Base const> const ref = holder;

    EXPECT_EQ(&ref.get(), &holder.base);
}

CTH_TEST(ref_wrapper, deduces_the_type) {
    Base base{EXAMPLE_NUMBER};
    Base const& constBase = base;

    ref_wrapper const ref{base};
    ref_wrapper const constRef{constBase};

    static_assert(std::same_as<decltype(ref), ref_wrapper<Base> const>);
    static_assert(std::same_as<decltype(constRef), ref_wrapper<Base const> const>);
}

CTH_TEST(ref_wrapper, arrow_and_star_reach_the_object) {
    Base base{EXAMPLE_NUMBER};
    ref_wrapper<Base> const ref{base};

    EXPECT_EQ(ref.operator->(), &base);
    EXPECT_EQ(&*ref, &base);

    ref->value = 1;
    EXPECT_EQ(base.value, 1);
    (*ref).value = 2;
    EXPECT_EQ(base.value, 2);
}

CTH_TEST(ref_wrapper, assignment_rebinds) {
    Base first{1};
    Base second{2};
    ref_wrapper<Base> ref{first};
    ref_wrapper<Base> const other{second};

    ref = other;
    EXPECT_EQ(&*ref, &second);

    ref = first;
    EXPECT_EQ(&*ref, &first);

    EXPECT_EQ(first.value, 1);
    EXPECT_EQ(second.value, 2);
}

CTH_TEST(ref_wrapper, constexpr) {
    static_assert(
        [] {
            Base base{EXAMPLE_NUMBER};
            ref_wrapper<Base> const ref{base};
            ref->value += 1;
            return (*ref).value;
        }()
        == EXAMPLE_NUMBER + 1
    );

    constexpr ref_wrapper constant{CONSTANT};
    static_assert(constant->value == EXAMPLE_NUMBER);
    static_assert(&*constant == &CONSTANT);
}

CTH_TEST(ref_wrapper, needs_an_object) {
    static_assert(!std::is_default_constructible_v<ref_wrapper<Base>>);
    static_assert(!std::is_constructible_v<ref_wrapper<Base>, Base&&>);
    static_assert(!std::is_constructible_v<ref_wrapper<Base>, Base const&>);
    static_assert(!std::is_constructible_v<ref_wrapper<Base>, Base*>);
}
}
