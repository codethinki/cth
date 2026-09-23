#include "test.hpp"

#include "cth/os/fence.hpp"

#include <chrono>
#include <thread>


namespace cth::os {
using namespace std::chrono_literals;

OS_TEST(fence, manual_reset) {
    fence fence;

    EXPECT_FALSE(fence.signaled());
    EXPECT_EQ(fence.wait(0ms), wait_result::TIMEOUT);

    fence.signal();

    EXPECT_TRUE(fence.signaled());
    EXPECT_EQ(fence.wait(0ms), wait_result::WAITED);
    EXPECT_EQ(fence.wait(0ms), wait_result::WAITED);

    fence.reset();

    EXPECT_FALSE(fence.signaled());
    EXPECT_EQ(fence.wait(0ms), wait_result::TIMEOUT);
}

OS_TEST(fence, wait_until_signaled) {
    fence fence;
    std::jthread signal{[&fence] { fence.signal(); }};

    EXPECT_EQ(fence.wait(100ms), wait_result::WAITED);
}

}
