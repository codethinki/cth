#include "cth/data/string_joiner.hpp"

#include "test.hpp"

#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cth::dt {

DATA_TEST(joiner, ConstructorsAndDelimiter) {
    string_joiner const sjDefault;
    ASSERT_EQ(sjDefault.delim(), " ");

    string_joiner const sjCustom("-->");
    ASSERT_EQ(sjCustom.delim(), "-->");

    string_joiner const sjReserve("--", 128);
    ASSERT_EQ(sjReserve.delim(), "--");
    ASSERT_GE(sjReserve.capacity(), 128);
}

DATA_TEST(joiner, OperatorPlusEquals) {
    string_joiner sj(", ");
    sj += "one";
    ASSERT_EQ(sj.string(), "one");
    sj += "two";
    ASSERT_EQ(sj.string(), "one, two");
    sj += 3;
    ASSERT_EQ(sj.string(), "one, two, 3");
}

DATA_TEST(joiner, OperatorPlus) {
    std::string const res1 = string_joiner(" | ") + "A" + "B" + "C";
    ASSERT_EQ(res1, "A | B | C");

    string_joiner sjBase(" ");
    sjBase += "base";
    auto const sjCopy = sjBase + "extended";
    ASSERT_EQ(sjBase.string(), "base");
    ASSERT_EQ(sjCopy.string(), "base extended");
}

DATA_TEST(joiner, OperatorEq) {
    auto const sj = string_joiner{" "} + "a" + "b" + "c";
    auto const sj2 = string_joiner{" "} + "a" + "b" + "c";
    std::string const expected = "a b c";

    EXPECT_EQ(sj, sj2);
    EXPECT_EQ(sj, expected);
}

DATA_TEST(joiner, ConversionsAndAccessors) {
    string_joiner sj(" ");
    sj += "test";
    sj += "string";
    std::string const expected = "test string";

    ASSERT_EQ(sj.string(), expected);
    ASSERT_EQ(std::move(sj).string(), expected);

    string_joiner sj2(" ");
    sj2 += "test";
    sj2 += "string";
    std::string const s = sj2;
    std::string_view const sv = sj2;
    ASSERT_EQ(s, expected);
    ASSERT_EQ(sv, expected);
}

DATA_TEST(joiner, Iterators) {
    string_joiner const sj = string_joiner(" ") + "a" + "b" + "c";
    std::string const expected = "a b c";

    std::string const fromFwdIter{std::from_range, sj};
    ASSERT_EQ(fromFwdIter, expected);

    std::string const fromRevIter{sj.rbegin(), sj.rend()};
    std::string expectedRev = expected;
    std::ranges::reverse(expectedRev);
    ASSERT_EQ(fromRevIter, expectedRev);
}

DATA_TEST(joiner, ElementAccess) {
    string_joiner const sj = string_joiner("") + "abc";
    std::string const expected = "abc";

    ASSERT_EQ(sj.front(), expected.front());
    ASSERT_EQ(sj.back(), expected.back());
    ASSERT_EQ(sj[1], expected[1]);
    ASSERT_EQ(sj.at(2), expected.at(2));
}

DATA_TEST(joiner, CapacityAndModifiers) {
    string_joiner sj(" ", 100);
    ASSERT_TRUE(sj.empty());
    ASSERT_GE(sj.capacity(), 100);

    sj += "word";
    ASSERT_FALSE(sj.empty());
    ASSERT_EQ(sj.size(), 4);
    ASSERT_EQ(sj.length(), 4);

    sj.clear();
    ASSERT_TRUE(sj.empty());
    ASSERT_EQ(sj.size(), 0);
}

DATA_TEST(joiner, SearchAndInspectMethods) {
    string_joiner const sj = string_joiner("/") + "path" + "to" + "file" + "file";
    std::string const expected = "path/to/file/file";

    ASSERT_EQ(sj.compare(expected), expected.compare(expected));

    ASSERT_EQ(sj.starts_with("path"), expected.starts_with("path"));
    ASSERT_FALSE(sj.starts_with("to"));
    ASSERT_EQ(sj.ends_with("file"), expected.ends_with("file"));
    ASSERT_TRUE(sj.contains("/to/"));

    ASSERT_EQ(sj.find("to"), expected.find("to"));
    ASSERT_EQ(sj.find('o'), expected.find('o'));
    ASSERT_EQ(sj.rfind('/'), expected.rfind('/'));
    ASSERT_EQ(sj.rfind("file"), expected.rfind("file"));
    ASSERT_EQ(sj.find_first_of("ot"), expected.find_first_of("ot"));
    ASSERT_EQ(sj.find_last_of("ot"), expected.find_last_of("ot"));
    ASSERT_EQ(sj.find_first_not_of("path/"), expected.find_first_not_of("path/"));
    ASSERT_EQ(sj.find_last_not_of("file/"), expected.find_last_not_of("file/"));

    ASSERT_EQ(sj.substr(5, 2), expected.substr(5, 2));
}
}
