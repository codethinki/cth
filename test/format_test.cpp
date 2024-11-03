#include "../cth/format.hpp"

#include <gtest/gtest.h>


namespace test {



enum Test1 {
    VALUE1,
};
enum Test2 {
    VALUE2
};




constexpr std::string_view to_string(Test1 e) {
    switch(e) {
        case VALUE1: return "VALUE1";
        default: return "unknown";
    }
}


template<class T>
concept test_concept = std::is_same_v<T, Test2>;
constexpr std::string_view to_string(Test2 e) {
    switch(e) {
        case VALUE2: return "VALUE2";
        default: return "unknown";
    }
}
}


CTH_FORMAT_TYPE(test::Test1, test::to_string);
CTH_FORMAT_CPT(test::test_concept, test::to_string);

namespace cth::fmt {
    TEST(cth_format_type, main) {
        auto const str = std::format("{}", test::VALUE1);

        ASSERT_EQ("test::Test1{VALUE1}", str);
    }
    TEST(cth_format_cpt, main) {
        auto const str = std::format("{}", test::VALUE2);
        ASSERT_EQ("test::test_concept{VALUE2}", str);
    }
}