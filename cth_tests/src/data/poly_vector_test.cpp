#include "test.hpp"

#include "cth/data/poly_vector.hpp"

#include <numeric>


namespace cth::dt {

// raw_poly_vector tests
DATA_TEST(raw_poly_vector, StaticAndTypeChecks) {
    using rv = cth::dt::raw_poly_vector<int, char, double, float>;

    EXPECT_EQ(rv::SIZE, 4);
    EXPECT_EQ(rv::N, 4);

    static_assert(std::same_as<rv::value_type<0>, int>, "value_type<0> should be int");
    static_assert(std::same_as<rv::value_type<1>, char>, "value_type<1> should be char");
    static_assert(std::same_as<rv::value_type<2>, double>, "value_type<2> should be double");
    static_assert(std::same_as<rv::value_type<3>, float>, "value_type<3> should be float");
}

DATA_TEST(raw_poly_vector, DataAndAlignmentAndIntegrity) {
    struct alignas(16) AlignedStruct {
        long long val;
        bool operator==(AlignedStruct const&) const = default;
    };

    using first_t = char;
    using second_t = AlignedStruct;
    using third_t = int;

    constexpr size_t n0 = 7;
    constexpr size_t n1 = 3;
    constexpr size_t n2 = 5;

    cth::dt::raw_poly_vector<first_t, second_t, third_t> rv(std::array{n0, n1, n2});

    auto p0 = rv.data<0>();
    auto p1 = rv.data<1>();
    auto p2 = rv.data<2>();

    auto const addr1 = reinterpret_cast<uintptr_t>(p1);
    EXPECT_EQ(addr1 % alignof(second_t), 0);

    auto const addr2 = reinterpret_cast<uintptr_t>(p2);
    EXPECT_EQ(addr2 % alignof(third_t), 0);

    std::fill(p0, p0 + n0, 'Z');
    p1[0] = {101};
    p1[1] = {202};
    p1[2] = {303};
    std::iota(p2, p2 + n2, -10);

    for(size_t i = 0; i < n0; ++i) { EXPECT_EQ(p0[i], 'Z'); }
    EXPECT_EQ(p1[0].val, 101);
    EXPECT_EQ(p1[1].val, 202);
    EXPECT_EQ(p1[2].val, 303);
    EXPECT_EQ(p2[0], -10);
    EXPECT_EQ(p2[4], -6);
}

DATA_TEST(raw_poly_vector, ConstData) {
    using first_t = long long;
    using second_t = char;

    constexpr size_t n0 = 5;
    constexpr size_t n1 = 8;

    cth::dt::raw_poly_vector<first_t, second_t> rv(std::array{n0, n1});

    auto p0 = rv.data<0>();
    auto p1 = rv.data<1>();
    std::iota(p0, p0 + n0, 100LL);
    std::iota(p1, p1 + n1, 'A');

    auto const& rvc = rv;
    auto cp0 = rvc.data<0>();
    auto cp1 = rvc.data<1>();
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*cp0)>>, "const& -> const pointer");
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*cp1)>>, "const& -> const pointer");

    EXPECT_EQ(cp0[0], 100LL);
    EXPECT_EQ(cp0[4], 104LL);
    EXPECT_EQ(cp1[0], 'A');
    EXPECT_EQ(cp1[7], 'H');
}
}


namespace cth::dt {

DATA_TEST(poly_vector, StaticAndTypeChecks) {
    using pv = cth::dt::poly_vector<int, char, double, float>;

    EXPECT_EQ(pv::SIZE, 4);
    EXPECT_EQ(pv::N, 4);

    static_assert(std::is_same_v<pv::value_type<0>, int>, "value_type<0> should be int");
    static_assert(std::is_same_v<pv::value_type<1>, char>, "value_type<1> should be char");
    static_assert(std::is_same_v<pv::value_type<2>, double>, "value_type<2> should be double");
    static_assert(std::is_same_v<pv::value_type<3>, float>, "value_type<3> should be float");
}

DATA_TEST(poly_vector, ConstructionAndSize) {
    using first_t = int;
    using second_t = char;
    using third_t = double;

    constexpr size_t n0 = 10;
    constexpr size_t n1 = 100;
    constexpr size_t n2 = 5;

    cth::dt::poly_vector<first_t, second_t, third_t> const pv(std::array{n0, n1, n2});

    EXPECT_EQ(pv.size<0>(), n0);
    EXPECT_EQ(pv.size<1>(), n1);
    EXPECT_EQ(pv.size<2>(), n2);
}

DATA_TEST(poly_vector, GetAndDataMutableConstStyle) {
    using first_t = long long;
    using second_t = char;

    constexpr size_t n0 = 5;
    constexpr size_t n1 = 8;

    cth::dt::poly_vector<first_t, second_t> pvMut(std::array{n0, n1});
    auto span0 = pvMut.get<0>();
    auto span1 = pvMut.get<1>();

    std::iota(span0.begin(), span0.end(), 100LL);
    std::iota(span1.begin(), span1.end(), 'A');

    EXPECT_EQ(span0.size(), n0);
    EXPECT_EQ(span1.size(), n1);

    auto const& pv = pvMut;
    auto cspan0 = pv.get<0>();
    auto cspan1 = pv.get<1>();
    static_assert(std::same_as<decltype(cspan0)::element_type, first_t const>, "const& -> const span");
    static_assert(std::same_as<decltype(cspan1)::element_type, second_t const>, "const& -> const span");

    EXPECT_EQ(cspan0[0], 100LL);
    EXPECT_EQ(cspan0[4], 104LL);
    EXPECT_EQ(cspan1[0], 'A');
    EXPECT_EQ(cspan1[7], 'H');
}

DATA_TEST(poly_vector, GetAndDataConst) {
    using first_t = int;
    using second_t = double;
    constexpr first_t firstDefault = 42;
    constexpr second_t secondDefault = 3.14;

    constexpr size_t firstSize = 3;
    constexpr size_t secondSize = 4;

    cth::dt::poly_vector<int, double> pvMut(std::initializer_list{firstSize, secondSize});
    auto span0 = pvMut.get<0>();
    auto span1 = pvMut.get<1>();
    // Populate the vector with some data first.
    std::ranges::fill(span0, firstDefault);
    std::ranges::fill(span1, secondDefault);

    EXPECT_EQ(span0.size(), firstSize);
    EXPECT_EQ(span1.size(), secondSize);

    // Create a const reference to the vector.
    auto const& pv = pvMut;
    auto cspan0 = pv.get<0>();
    auto cspan1 = pv.get<1>();
    static_assert(std::same_as<typename decltype(cspan0)::element_type, first_t const>, "const& -> const span");
    static_assert(std::same_as<typename decltype(cspan1)::element_type, second_t const>, "const& -> const span");
    EXPECT_EQ(cspan0.size(), firstSize);
    EXPECT_EQ(cspan1.size(), secondSize);

    for(auto const& x : span0)
        EXPECT_EQ(x, firstDefault);
    for(auto const& x : span1)
        EXPECT_DOUBLE_EQ(x, secondDefault);
}

DATA_TEST(poly_vector, ZeroSizedSpans) {
    using t0 = int;
    using t1 = char;
    using t2 = double;
    using t3 = float;

    constexpr size_t n0 = 0;
    constexpr size_t n1 = 10;
    constexpr size_t n2 = 0;
    constexpr size_t n3 = 5;

    cth::dt::poly_vector<t0, t1, t2, t3> pvMut(std::array{n0, n1, n2, n3});

    EXPECT_EQ(pvMut.size<0>(), n0);
    EXPECT_TRUE(pvMut.get<0>().empty());
    EXPECT_NE(pvMut.data<0>(), nullptr);

    EXPECT_EQ(pvMut.size<1>(), n1);
    EXPECT_FALSE(pvMut.get<1>().empty());

    EXPECT_EQ(pvMut.size<2>(), n2);
    EXPECT_TRUE(pvMut.get<2>().empty());
    EXPECT_NE(pvMut.data<2>(), nullptr);

    EXPECT_EQ(pvMut.size<3>(), n3);
    EXPECT_FALSE(pvMut.get<3>().empty());

    auto const& pv = pvMut;
    EXPECT_TRUE(pv.get<0>().empty());
    EXPECT_FALSE(pv.get<1>().empty());
    EXPECT_TRUE(pv.get<2>().empty());
    EXPECT_FALSE(pv.get<3>().empty());
}

DATA_TEST(poly_vector, AlignmentAndDataIntegrity) {
    struct alignas(16) AlignedStruct {
        long long val;
        bool operator==(AlignedStruct const&) const = default;
    };

    using pv_t = cth::dt::poly_vector<char, AlignedStruct, int>;
    constexpr size_t n0 = 7, n1 = 3, n2 = 5;
    pv_t pvMut(std::array{n0, n1, n2});

    auto const ptrAddrStruct = reinterpret_cast<uintptr_t>(pvMut.data<1>());
    EXPECT_EQ(ptrAddrStruct % alignof(AlignedStruct), 0);

    auto const ptrAddrInt = reinterpret_cast<uintptr_t>(pvMut.data<2>());
    EXPECT_EQ(ptrAddrInt % alignof(int), 0);

    std::fill(pvMut.get<0>().begin(), pvMut.get<0>().end(), 'Z');
    pvMut.get<1>()[0] = {101};
    pvMut.get<1>()[1] = {202};
    pvMut.get<1>()[2] = {303};
    std::iota(pvMut.get<2>().begin(), pvMut.get<2>().end(), -10);

    auto const& pv = pvMut;

    for(char val : pv.get<0>()) { EXPECT_EQ(val, 'Z'); }

    auto const s1 = pv.get<1>();
    EXPECT_EQ(s1[0].val, 101);
    EXPECT_EQ(s1[1].val, 202);
    EXPECT_EQ(s1[2].val, 303);

    auto const s2 = pv.get<2>();
    EXPECT_EQ(s2[0], -10);
    EXPECT_EQ(s2[4], -6);
}

DATA_TEST(poly_vector, copy) {
    using base_t = poly_vector<int, double, float>;

    base_t base{{1, 2, 3}};

    base.get<0>()[0] = 42;
    std::ranges::fill(base.get<1>(), 3.14);
    std::ranges::fill(base.get<2>(), 2.71f);

    poly_vector copyCtor{base};

    auto check_equal = [](base_t const& actual, base_t const& expected) {
        ASSERT_TRUE(std::ranges::equal(actual.sizes(), expected.sizes()));
        ASSERT_TRUE(std::ranges::equal(actual.get<0>(), expected.get<0>()));
        ASSERT_TRUE(std::ranges::equal(actual.get<1>(), expected.get<1>()));
        ASSERT_TRUE(std::ranges::equal(actual.get<2>(), expected.get<2>()));
    };

    check_equal(copyCtor, base);

    base_t copyAssign{{1, 2, 3}};
    copyAssign = base;
    check_equal(copyAssign, base);


}

}
