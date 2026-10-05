#include "test.hpp"

#include "cth/data/cxpr.hpp"

#include <string_view>
#include <vector>

namespace cth::dt {

DATA_TEST(as_cxpr_array, main) {
    constexpr auto array = as_cxpr_array<[] { return std::vector{1, 2, 3}; }>();

    static_assert(array.size() == 3);
    EXPECT_EQ(array, (std::array{1, 2, 3}));
}

DATA_TEST(as_cxpr_array, computed_size) {
    static constexpr std::string_view CSV = "a,bb,ccc";

    constexpr auto splits = as_cxpr_array<[] {
        std::vector<size_t> result{};
        for(size_t i = 0; i < CSV.size(); i++)
            if(CSV[i] == ',')
                result.push_back(i);
        return result;
    }>();

    EXPECT_EQ(splits, (std::array<size_t, 2>{1, 4}));
}

}
