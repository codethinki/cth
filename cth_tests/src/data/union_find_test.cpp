#include "test.hpp"

#include "cth/data/union_find.hpp"

#include "cth/numeric.hpp"

#include <random>



namespace cth::dt {
DATA_TEST(union_find, gt1_chain) {
    union_find uf(3);

    // after these merges the internal parent array becomes:
    // merge(0,1) -> parent(0) = 1
    // merge(1,2) -> parent(1) = 2  (so 0 -> 1 -> 2)
    uf.merge(0, 1);
    uf.merge(1, 2);

    // proven by the chain_length inspector
    EXPECT_GE(2u, uf.chain_length(0)); // chain length should be > 1

    uf.find(0);
    EXPECT_GE(1u, uf.chain_length(0));
}

DATA_TEST(union_find, ctors) {
    static constexpr size_t SIZE = 5;

    union_find sizeCtor(SIZE);
    ASSERT_EQ(sizeCtor.size(), SIZE);
    for(size_t i = 0; i < SIZE; i++)
        ASSERT_TRUE(sizeCtor.root(i));

    static std::array<std::unordered_set<size_t>, SIZE> groups{
        {
            {0, 1, 2},
            {3},
        }
    };


    union_find spanSizeCtor(SIZE, groups);
    union_find initListSizeCtor(SIZE, {{0, 1, 2}, {3}});

    auto check_groups = [](union_find const& find, size_t size) {
        ASSERT_EQ(find.size(), size);
        for(auto& index : groups[0])
            ASSERT_EQ(find.find(index), *groups[0].begin());
        for(auto& index : groups[1])
            ASSERT_EQ(find.find(index), *groups[1].begin());
        ASSERT_NE(find.find(*groups[0].begin()), find.find(*groups[1].begin()));
    };

    check_groups(spanSizeCtor, SIZE);
    check_groups(initListSizeCtor, SIZE);

    union_find implicitSizeSpanCtor(groups);
    union_find implicitSizeInitListCtor({{0, 1, 2}, {3}});

    check_groups(implicitSizeSpanCtor, SIZE - 1);
    check_groups(implicitSizeInitListCtor, SIZE - 1);
}

}
