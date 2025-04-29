#include "test.hpp"

#include "cth/types/md.hpp"

namespace cth::type {
TYPE_TEST(mdextent, compile) {
    constexpr size_t rank = 3;
    constexpr size_t extent = 5;

    auto const result = mdextent<rank, extent>();


    EXPECT_EQ(result.rank(), rank);
    for(size_t i = 0; i < rank; i++) {
        EXPECT_EQ(result.static_extent(i), extent);
        EXPECT_EQ(result.extent(i), extent);
    }
}

TYPE_TEST(mdextent_t, compile) {
    constexpr size_t rank = 3;
    constexpr size_t extent = 5;
    using result_t = sq_mdextent_t<rank, extent>;
    EXPECT_EQ(result_t::rank(), rank);
    for(size_t i = 0; i < rank; i++) {
        EXPECT_EQ(result_t::static_extent(i), extent);
    }
}

TYPE_TEST(mdspan_t, compile) {
    constexpr size_t rank = 3;
    constexpr size_t extent = 5;
    using result_t = sq_mdspan_t<char const, rank, extent>;
    EXPECT_EQ(result_t::rank(), rank);
    for(size_t i = 0; i < rank; i++) {
        EXPECT_EQ(result_t::static_extent(i), extent);
    }

    std::vector<char> example(125);

    [[maybe_unused]] result_t span{example.data(), extent, extent, extent};
}

} //namespace cth::type
