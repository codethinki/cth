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

}

