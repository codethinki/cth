#include "cth/test.hpp"

#define FMT_TEST(suite, test_name) CTH_EX_TEST(format, suite, test_name)

#include "cth/cth.hpp"
#include "cth/string/format.hpp"

#include <string>
#include <vector>


namespace test {


struct Test1 {
    int x;
    int y;
};
enum Test2 { VALUE2 };



constexpr std::string_view to_string(Test2 const& e) {
    switch(e) {
        case VALUE2: return "VALUE2";
        default: return "unknown";
    }
}

struct Test3 {
    Test1 a;
};

template<class T>
concept test_concept = std::same_as<T, Test3>;
}


CTH_FORMAT_CLASS(test::Test1, "x: {}, y: {}", ([](test::Test1 const& test) {return std::tie(test.x, test.y); }));
CTH_FORMAT_CLASS_CUSTOM(test::Test2, "{}", test::to_string);

namespace test {
std::string to_string(Test3 const& e) { return std::format("{}", e.a); }
}

CTH_FORMAT_CPT(test::test_concept, test::to_string);


namespace cth::fmt {
::test::Test1 x{1, 2};

FMT_TEST(CTH_FORMAT_CLASS_CUSTOM, main) {
    auto const str = std::format("{}", ::test::VALUE2);
    ASSERT_EQ(str, "VALUE2");
}

FMT_TEST(CTH_FORMAT_CLASS, main) {
    auto const str = std::format("{}", x);
    auto const expected = std::format("test::Test1{{x: {}, y: {}}}", x.x, x.y);
    ASSERT_EQ(str, expected);
}
FMT_TEST(CTH_FORMAT_CPT, main) {
    auto const str = std::format("{}", ::test::Test3{x});
    auto const expected = std::format("test::test_concept{{{}}}", x);

    ASSERT_EQ(str, expected);
}

FMT_TEST(range, main) {
    std::vector<::test::Test1> const points{{1, 2}, {3, 4}};
    std::vector<std::string> const words{"oaf", "farthest"};
    std::vector<std::vector<int>> const nested{{1, 2}, {3}};

    EXPECT_EQ(std::format("{}", points), "[test::Test1{x: 1, y: 2}, test::Test1{x: 3, y: 4}]");
    EXPECT_EQ(std::format("{}", words), R"(["oaf", "farthest"])");
    EXPECT_EQ(std::format("{::}", words), "[oaf, farthest]");
    EXPECT_EQ(std::format("{}", nested), "[[1, 2], [3]]");
}
}
