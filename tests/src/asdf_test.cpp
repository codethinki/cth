#include "../../out/build/win_msvc_debug/vcpkg_installed/x64-windows/include/gtest/gtest.h"

#include "cth/types/typ_ranges.hpp"

#include <vector>
#include <array>

namespace cth {
TEST(asdf, asdf) {
    static_assert(cth::type::range2d_over_cpt<std::vector<std::vector<unsigned long long>>, CPT(std::integral)>, "fuck");
}

}
