#include "test.hpp"

#include "cth/os/native_handle.hpp"
#include "cth/os/timer.hpp"


namespace cth::os {

OS_TEST(unique_handle, empty_by_default) {
    unique_native_handle h{};

    EXPECT_FALSE(static_cast<bool>(h));
    EXPECT_EQ(h.get(), invalid_handle());
}

// guards the posix trap: fd 0 is stdin, so the empty state cannot be the zero value
OS_TEST(unique_handle, invalid_is_not_zero_on_posix) {
#ifdef CTH_FS_POSIX
    EXPECT_EQ(invalid_handle(), -1);
#else
    EXPECT_EQ(invalid_handle(), nullptr);
#endif
}

OS_TEST(unique_handle, owns_after_release) {
    timer t{}; // borrow a real handle rather than inventing one
    unique_native_handle h{t.native_handle()};

    EXPECT_TRUE(static_cast<bool>(h));

    auto const raw = h.release();

    EXPECT_EQ(raw, t.native_handle());
    EXPECT_FALSE(static_cast<bool>(h));
}

OS_TEST(unique_handle, move_leaves_source_empty) {
    timer t{};
    unique_native_handle src{t.native_handle()};

    unique_native_handle dst{std::move(src)};

    EXPECT_TRUE(static_cast<bool>(dst));
    EXPECT_FALSE(static_cast<bool>(src));

    [[maybe_unused]] auto const leaked = dst.release(); // the timer still owns it
}

}
