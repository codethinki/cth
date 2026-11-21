#include "cth/test.hpp"

#include "cth/io/axis.hpp"

#include <format>


namespace cth::io {
namespace {
    /**
     * `true` if `to_utf8_string()` has a symbol for every `Axis`
     * @details a missing case hits `std::unreachable()`, no constant expression
     */
    consteval bool every_axis_has_a_symbol() {
        for(size_t i = 0; i < axes_size(); ++i)
            if(to_utf8_string(static_cast<Axis>(i)).empty())
                return false;
        return true;
    }
}

CTH_TEST(Axis, to_utf8_string_has_a_symbol_for_every_axis) { static_assert(every_axis_has_a_symbol()); }

CTH_TEST(Axis, axes_size_counts_every_axis) {
    static_assert(axes_size() == *Axis::ENUM_LAST_VALUE + 1u);
    static_assert(*Axis::NONE == 0);
}

CTH_TEST(axis_state, equality_compares_axis_and_value) {
    constexpr axis_state state{Axis::MOUSE_POSITION, {1.f, 2.f}};

    static_assert(state == axis_state{Axis::MOUSE_POSITION, {1.f, 2.f}});
    static_assert(state != axis_state{Axis::MOUSE_SCROLL, {1.f, 2.f}});
    static_assert(state != axis_state{Axis::MOUSE_POSITION, {1.f, 3.f}});
}

CTH_TEST(axis_update, equality_compares_data_and_time) {
    auto const time = chrono::clock_t::now();
    axis_update const update{{Axis::MOUSE_SCROLL, {1.f, 0.f}}, time};

    EXPECT_EQ(update, (axis_update{{Axis::MOUSE_SCROLL, {1.f, 0.f}}, time}));
    EXPECT_NE(update, (axis_update{{Axis::MOUSE_SCROLL, {2.f, 0.f}}, time}));
    EXPECT_NE(update, (axis_update{{Axis::MOUSE_SCROLL, {1.f, 0.f}}, time + std::chrono::seconds{1}}));
}

CTH_TEST(axis_state, format_prints_axis_and_value) {
    axis_state const state{Axis::MOUSE_POSITION, {1.f, 2.f}};

    EXPECT_EQ(std::format("{}", state), "cth::io::axis_state{cth::io::Axis{MousePos}, value: [1, 2]}");
}
}
