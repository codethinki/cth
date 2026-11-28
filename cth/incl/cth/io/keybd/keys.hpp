#pragma once
#include "cth/chrono.hpp"

#include "cth/enums.hpp"
#include "cth/macro.hpp"
#include "cth/os/osdef.hpp"
#include "cth/string/format.hpp"
#include "cth/types.hpp"

#include <cstddef>
#include <chrono>
#include <string>
#include <utility>

namespace cth::io {
/**
 * physical position of a key, named after the US layout
 * @details
 * - like `KeyboardEvent.code` of browsers
 * - the output of the key in the layout is `key_state::character`
 * - comparison operators may be used for symbol ranges, see implementation for supported ones
 * - `operator*()` converts to the underlying type
 */
enum class Key : u16 {
    NONE, // No key, or invalid key (keep this near the end for logical grouping)

    // --- Alphabetical Keys (A-Z) ---
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    // --- Numerical Keys (0-9) ---
    DIGIT_0,
    DIGIT_1,
    DIGIT_2,
    DIGIT_3,
    DIGIT_4,
    DIGIT_5,
    DIGIT_6,
    DIGIT_7,
    DIGIT_8,
    DIGIT_9,
    // --- Symbol Keys (Common Symbols, somewhat grouped by location on keyboard) ---
    SPACE,
    BACKTICK, // ` ~
    MINUS, // - _
    EQUAL, // = +
    OPEN_BRACKET, // [ {
    CLOSE_BRACKET, // ] }
    BACKSLASH, // \ | (ISO: the key left of Enter)
    SEMICOLON, // ; :
    QUOTE, // ' "
    COMMA, // , <
    PERIOD, // . >
    SLASH, // / ?
    PLUS, ///< keypad `+` only, with `ex_key::right`
    STAR, ///< keypad `*` only, with `ex_key::right`
    LESS_THAN, // < (extra ISO key between left Shift and Z)

    // --- Navigation and Editing Keys (No Left/Right Specifics) ---
    BACKSPACE,
    CAPS_LOCK,
    DEL, ///< Delete, not `DELETE`: `windows.h` defines `DELETE` as a macro
    END,
    ENTER, // Return/Enter
    ESCAPE,
    HOME,
    INSERT,
    PAGE_DOWN,
    PAGE_UP,
    TAB,
    // --- Arrow Keys ---
    ARROW_DOWN,
    ARROW_LEFT,
    ARROW_RIGHT,
    ARROW_UP,
    // --- Modifier Keys (Left/Right handled separately in adapter logic) ---
    ALT, // General Alt (Left or Right, disambiguation in handling)
    CONTROL, // General Control (Left or Right, disambiguation in handling)
    SHIFT, // General Shift (Left or Right, disambiguation in handling)

    // --- Meta and Platform Keys ---
    META, // Generic Meta key (for layers, custom functions)
    PLATFORM, // OS Specific Platform key (Win/Cmd/Super)

    // --- Function Keys (F1-F12) ---
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    NUM_LOCK,
    // --- Function Keys (F13-F24) ---
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    // --- System Keys ---
    PRINT_SCREEN,
    SCROLL_LOCK,
    PAUSE,
    MENU,
    // --- Mouse Buttons ---
    MOUSE_LEFT,
    MOUSE_RIGHT,
    MOUSE_MIDDLE,
    MOUSE_4,
    MOUSE_5,
    // --- Custom Keys (User-Defined Functions or Layers) ---
    CUSTOM_0,
    CUSTOM_1,
    CUSTOM_2,
    CUSTOM_3,
    CUSTOM_4,
    ENUM_LAST_VALUE = CUSTOM_4 // Keep this LAST - for array sizing, iteration, etc.
};

consteval auto keys_size() { return static_cast<uz>(Key::ENUM_LAST_VALUE) + 1; }

/**
 * converts the Key to UTF-8 string symbol
 */
constexpr std::string_view to_utf8_string(Key key) {
    switch(key) {
        // Alphabetical keys
        case Key::A: return "A";
        case Key::B: return "B";
        case Key::C: return "C";
        case Key::D: return "D";
        case Key::E: return "E";
        case Key::F: return "F";
        case Key::G: return "G";
        case Key::H: return "H";
        case Key::I: return "I";
        case Key::J: return "J";
        case Key::K: return "K";
        case Key::L: return "L";
        case Key::M: return "M";
        case Key::N: return "N";
        case Key::O: return "O";
        case Key::P: return "P";
        case Key::Q: return "Q";
        case Key::R: return "R";
        case Key::S: return "S";
        case Key::T: return "T";
        case Key::U: return "U";
        case Key::V: return "V";
        case Key::W: return "W";
        case Key::X: return "X";
        case Key::Y: return "Y";
        case Key::Z: return "Z";

        // Numerical keys
        case Key::DIGIT_0: return "0";
        case Key::DIGIT_1: return "1";
        case Key::DIGIT_2: return "2";
        case Key::DIGIT_3: return "3";
        case Key::DIGIT_4: return "4";
        case Key::DIGIT_5: return "5";
        case Key::DIGIT_6: return "6";
        case Key::DIGIT_7: return "7";
        case Key::DIGIT_8: return "8";
        case Key::DIGIT_9: return "9";

        // Symbol keys
        case Key::SPACE: return "⎵";
        case Key::BACKTICK: return "`";
        case Key::MINUS: return "−";
        case Key::EQUAL: return "=";
        case Key::OPEN_BRACKET: return "[";
        case Key::CLOSE_BRACKET: return "]";
        case Key::BACKSLASH: return "\\";
        case Key::SEMICOLON: return ";";
        case Key::QUOTE: return "\'";
        case Key::COMMA: return ",";
        case Key::PERIOD: return ".";
        case Key::SLASH: return "/";
        case Key::PLUS: return "+";
        case Key::STAR: return "*";
        case Key::LESS_THAN: return "<";

        // Navigation and editing keys
        case Key::BACKSPACE: return "⌫";
        case Key::CAPS_LOCK: return "⇪";
        case Key::DEL: return "Del";
        case Key::END: return "End";
        case Key::ENTER: return "↵";
        case Key::ESCAPE: return "Esc";
        case Key::HOME: return "Home";
        case Key::INSERT: return "Ins";
        case Key::PAGE_DOWN: return "⇟";
        case Key::PAGE_UP: return "⇞";
        case Key::TAB: return "⇥";

        // Arrow keys
        case Key::ARROW_DOWN: return "↓";
        case Key::ARROW_LEFT: return "←";
        case Key::ARROW_RIGHT: return "→";
        case Key::ARROW_UP: return "↑";

        // Modifier keys
        case Key::ALT: return "⌥";
        case Key::CONTROL: return "Ctrl";
        case Key::SHIFT: return "⇧";

        // Meta and platform keys
        case Key::META: return "Meta";
#ifdef CTH_PLATFORM_WINDOWS
        case Key::PLATFORM: return "WIN";
#else
        case Key::PLATFORM: return "Super";
#endif

        // Function keys
        case Key::F1: return "F1";
        case Key::F2: return "F2";
        case Key::F3: return "F3";
        case Key::F4: return "F4";
        case Key::F5: return "F5";
        case Key::F6: return "F6";
        case Key::F7: return "F7";
        case Key::F8: return "F8";
        case Key::F9: return "F9";
        case Key::F10: return "F10";
        case Key::F11: return "F11";
        case Key::F12: return "F12";
        case Key::F13: return "F13";
        case Key::F14: return "F14";
        case Key::F15: return "F15";
        case Key::F16: return "F16";
        case Key::F17: return "F17";
        case Key::F18: return "F18";
        case Key::F19: return "F19";
        case Key::F20: return "F20";
        case Key::F21: return "F21";
        case Key::F22: return "F22";
        case Key::F23: return "F23";
        case Key::F24: return "F24";

        case Key::NUM_LOCK: return "NumLock";

        // System keys
        case Key::PRINT_SCREEN: return "PrtSc";
        case Key::SCROLL_LOCK: return "ScrLk";
        case Key::PAUSE: return "Pause";
        case Key::MENU: return "Menu";

        // Mouse buttons
        case Key::MOUSE_LEFT: return "LMB";
        case Key::MOUSE_RIGHT: return "RMB";
        case Key::MOUSE_MIDDLE: return "MMB";
        case Key::MOUSE_4: return "M4";
        case Key::MOUSE_5: return "M5";

        // Custom keys
        case Key::CUSTOM_0: return "Custom0";
        case Key::CUSTOM_1: return "Custom1";
        case Key::CUSTOM_2: return "Custom2";
        case Key::CUSTOM_3: return "Custom3";
        case Key::CUSTOM_4: return "Custom4";

        // Invalid/none key
        case Key::NONE: return "∅";

        default: std::unreachable();
    }
}

}

CTH_GEN_ENUM_DEREF_OVERLOAD(cth::io::Key)

CTH_FORMAT_CLASS(
    cth::io::Key,
    "{}",
    ([](cth::io::Key const& key) { return cth::io::to_utf8_string(key); })
);

namespace cth::io {

struct ex_key {
    Key key;
    /**
     * right copy of a key that exists twice
     * @details
     * - modifier: right modifier (`SHIFT`, `CONTROL`, `ALT`, `PLATFORM`)
     * - other key: keypad copy, the keypad is right of the main block
     * - examples: `DIGIT_0` -> keypad 0, `ENTER` -> keypad Enter
     * - keypad copy by position: NumLock off does not change it
     */
    bool right = false;

    [[nodiscard]] constexpr bool operator==(ex_key const&) const = default;
};

struct key_state {
    constexpr key_state(ex_key exKey, bool down, std::string character = {}, bool repeat = false) :
        exKey{exKey}, down{down}, character{std::move(character)}, repeat{repeat} {}
    constexpr key_state(Key key, bool right, bool down, std::string character = {}, bool repeat = false) :
        key_state{{key, right}, down, std::move(character), repeat} {}

    ex_key exKey;
    bool down;
    /**
     * UTF-8 output of the key in the layout without modifiers, empty -> none
     * @details control characters, the same on every platform:
     * - Enter `\r`, Tab `\t`, Backspace `\b`, Escape `\x1B`, Delete `\x7F`
     */
    std::string character{};
    bool repeat = false; ///< `true` -> auto repeat, a down event of a key that is already down

    [[nodiscard]] constexpr bool operator==(key_state const&) const = default;
};

struct key_update {
    key_state data;
    chrono::time_point_t time;

    [[nodiscard]] constexpr bool operator==(key_update const&) const = default;
};

}

CTH_FORMAT_CLASS(
    cth::io::ex_key,
    "{}, right: {}",
    ([](cth::io::ex_key const& key) { return std::tie(key.key, key.right); })
);

CTH_FORMAT_CLASS(
    cth::io::key_state,
    "{}, down: {}, character: {:?}, repeat: {}",
    (
        [](cth::io::key_state const& key) {
            return std::tie(key.exKey, key.down, key.character, key.repeat);
        }
    )
);

CTH_FORMAT_CLASS(
    cth::io::key_update,
    "{}, time: {}",
    (
        [](cth::io::key_update const& update) {
            return std::make_tuple(
                update.data,
                std::chrono::duration_cast<std::chrono::seconds>(update.time - cth::chrono::session_start)
            );
        }
    )
);
