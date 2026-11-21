#pragma once
#include "cth/chrono.hpp"

#include "cth/enums.hpp"
#include "cth/string/format.hpp"

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>

namespace cth::io {
/**
 * axis of an input device
 * @details `operator*()` converts to the underlying type
 */
enum class Axis : uint16_t {
    NONE, ///< no axis, or invalid axis

    // --- Mouse Axes ---
    MOUSE_POSITION, ///< absolute, pixels in the window
    MOUSE_SCROLL, ///< relative, steps since the last update

    // --- Custom Axes (User-Defined Functions or Layers) ---
    CUSTOM_0,
    CUSTOM_1,
    CUSTOM_2,
    CUSTOM_3,
    CUSTOM_4,
    ENUM_LAST_VALUE = CUSTOM_4 ///< keep this last, for array sizes and iteration
};

consteval auto axes_size() { return static_cast<size_t>(Axis::ENUM_LAST_VALUE) + 1; }

/**
 * converts the `Axis` to a UTF-8 string symbol
 */
constexpr std::string_view to_utf8_string(Axis axis) {
    switch(axis) {
        // Mouse axes
        case Axis::MOUSE_POSITION: return "MousePos";
        case Axis::MOUSE_SCROLL: return "MouseScroll";

        // Custom axes
        case Axis::CUSTOM_0: return "Custom0";
        case Axis::CUSTOM_1: return "Custom1";
        case Axis::CUSTOM_2: return "Custom2";
        case Axis::CUSTOM_3: return "Custom3";
        case Axis::CUSTOM_4: return "Custom4";

        // Invalid/none axis
        case Axis::NONE: return "∅";

        default: std::unreachable();
    }
}

}

CTH_GEN_ENUM_DEREF_OVERLOAD(cth::io::Axis)

CTH_FORMAT_CLASS(
    cth::io::Axis,
    "{}",
    ([](cth::io::Axis const& axis) { return cth::io::to_utf8_string(axis); })
);

namespace cth::io {

struct axis_state {
    Axis axis;
    std::array<float, 2> value; ///< a 1D axis uses `value[0]`

    [[nodiscard]] constexpr bool operator==(axis_state const&) const = default;
};

struct axis_update {
    axis_state data;
    chrono::time_point_t time;

    [[nodiscard]] constexpr bool operator==(axis_update const&) const = default;
};

}

CTH_FORMAT_CLASS(
    cth::io::axis_state,
    "{}, value: {}",
    ([](cth::io::axis_state const& state) { return std::tie(state.axis, state.value); })
);

CTH_FORMAT_CLASS(
    cth::io::axis_update,
    "{}, time: {}",
    (
        [](cth::io::axis_update const& update) {
            return std::make_tuple(
                update.data,
                std::chrono::duration_cast<std::chrono::seconds>(update.time - cth::chrono::session_start)
            );
        }
    )
);
