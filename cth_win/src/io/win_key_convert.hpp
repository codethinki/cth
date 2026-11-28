#pragma once
#include "cth/chrono.hpp"
#include "cth/io/log.hpp"
#include "cth/win/io/keyboard.hpp"
#include "cth/win/string.hpp"

// private
#include "../win_include.hpp"
#include "cth/types.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>


namespace cth::win {
using scan_code_t = u16; ///< Set 1 scan code, the prefix `0xE0` or `0xE1` in the high byte: `0xE01C`

struct scan_code_key {
    scan_code_t scanCode;
    io::ex_key exKey;
};

/**
 * Set 1 scan codes of the keys
 * @details
 * - one entry per key and per scan code: `to_scan_code()` and `to_ex_key()` read it in both directions
 * - the scan code is the position, `0x2B` is the US `\` and the ISO `#`
 * - missing: the fake Shift `0xE02A` and `0xE036` of raw input, media keys, keys without an `io::Key`
 */
inline constexpr auto SCAN_CODE_KEYS = [] {
    using enum io::Key;

    return std::to_array<scan_code_key>(
        {
            // --- Main Block ---
            {0x01, {ESCAPE}},
            {0x02, {DIGIT_1}},
            {0x03, {DIGIT_2}},
            {0x04, {DIGIT_3}},
            {0x05, {DIGIT_4}},
            {0x06, {DIGIT_5}},
            {0x07, {DIGIT_6}},
            {0x08, {DIGIT_7}},
            {0x09, {DIGIT_8}},
            {0x0A, {DIGIT_9}},
            {0x0B, {DIGIT_0}},
            {0x0C, {MINUS}},
            {0x0D, {EQUAL}},
            {0x0E, {BACKSPACE}},
            {0x0F, {TAB}},
            {0x10, {Q}},
            {0x11, {W}},
            {0x12, {E}},
            {0x13, {R}},
            {0x14, {T}},
            {0x15, {Y}},
            {0x16, {U}},
            {0x17, {I}},
            {0x18, {O}},
            {0x19, {P}},
            {0x1A, {OPEN_BRACKET}},
            {0x1B, {CLOSE_BRACKET}},
            {0x1C, {ENTER}},
            {0x1D, {CONTROL}},
            {0x1E, {A}},
            {0x1F, {S}},
            {0x20, {D}},
            {0x21, {F}},
            {0x22, {G}},
            {0x23, {H}},
            {0x24, {J}},
            {0x25, {K}},
            {0x26, {L}},
            {0x27, {SEMICOLON}},
            {0x28, {QUOTE}},
            {0x29, {BACKTICK}},
            {0x2A, {SHIFT}},
            {0x2B, {BACKSLASH}},
            {0x2C, {Z}},
            {0x2D, {X}},
            {0x2E, {C}},
            {0x2F, {V}},
            {0x30, {B}},
            {0x31, {N}},
            {0x32, {M}},
            {0x33, {COMMA}},
            {0x34, {PERIOD}},
            {0x35, {SLASH}},
            {0x36, {SHIFT, true}},
            {0x38, {ALT}},
            {0x39, {SPACE}},
            {0x3A, {CAPS_LOCK}},
            {0x56, {LESS_THAN}},

            // --- Function Keys ---
            {0x3B, {F1}},
            {0x3C, {F2}},
            {0x3D, {F3}},
            {0x3E, {F4}},
            {0x3F, {F5}},
            {0x40, {F6}},
            {0x41, {F7}},
            {0x42, {F8}},
            {0x43, {F9}},
            {0x44, {F10}},
            {0x57, {F11}},
            {0x58, {F12}},
            {0x64, {F13}},
            {0x65, {F14}},
            {0x66, {F15}},
            {0x67, {F16}},
            {0x68, {F17}},
            {0x69, {F18}},
            {0x6A, {F19}},
            {0x6B, {F20}},
            {0x6C, {F21}},
            {0x6D, {F22}},
            {0x6E, {F23}},
            {0x76, {F24}},

            // --- System Keys ---
            {0xE037, {PRINT_SCREEN}},
            {0x46, {SCROLL_LOCK}},
            {0xE11D, {PAUSE}},
            {0x45, {NUM_LOCK}},

            // --- Modifiers ---
            {0xE01D, {CONTROL, true}},
            {0xE038, {ALT, true}},
            {0xE05B, {PLATFORM}},
            {0xE05C, {PLATFORM, true}},
            {0xE05D, {MENU}},

            // --- Navigation Block ---
            {0xE047, {HOME}},
            {0xE048, {ARROW_UP}},
            {0xE049, {PAGE_UP}},
            {0xE04B, {ARROW_LEFT}},
            {0xE04D, {ARROW_RIGHT}},
            {0xE04F, {END}},
            {0xE050, {ARROW_DOWN}},
            {0xE051, {PAGE_DOWN}},
            {0xE052, {INSERT}},
            {0xE053, {DEL}},

            // --- Keypad, by position: NumLock changes the meaning, not the key ---
            {0x37, {STAR, true}},
            {0x47, {DIGIT_7, true}},
            {0x48, {DIGIT_8, true}},
            {0x49, {DIGIT_9, true}},
            {0x4A, {MINUS, true}},
            {0x4B, {DIGIT_4, true}},
            {0x4C, {DIGIT_5, true}},
            {0x4D, {DIGIT_6, true}},
            {0x4E, {PLUS, true}},
            {0x4F, {DIGIT_1, true}},
            {0x50, {DIGIT_2, true}},
            {0x51, {DIGIT_3, true}},
            {0x52, {DIGIT_0, true}},
            {0x53, {PERIOD, true}},
            {0x59, {EQUAL, true}},
            {0xE01C, {ENTER, true}},
            {0xE035, {SLASH, true}},
        }
    );
}();

/**
 * Set 1 scan code of the key
 * @return `0` -> no scan code
 */
[[nodiscard]] constexpr scan_code_t to_scan_code(io::ex_key exKey) {
    auto const it = std::ranges::find(SCAN_CODE_KEYS, exKey, &scan_code_key::exKey);
    return it == SCAN_CODE_KEYS.end() ? 0 : it->scanCode;
}

/**
 * key of the Set 1 scan code
 * @return `Key::NONE` -> no key
 */
[[nodiscard]] constexpr io::ex_key to_ex_key(scan_code_t scanCode) {
    auto const it = std::ranges::find(SCAN_CODE_KEYS, scanCode, &scan_code_key::scanCode);
    return it == SCAN_CODE_KEYS.end() ? io::ex_key{io::Key::NONE} : it->exKey;
}

/**
 * Set 1 scan code of the raw input
 */
[[nodiscard]] constexpr scan_code_t to_scan_code(RAWKEYBOARD const& raw) {
    bool const e0 = (raw.Flags & RI_KEY_E0) != 0;
    bool const e1 = (raw.Flags & RI_KEY_E1) != 0;

    scan_code_t const prefix = e0 ? 0xE000 : e1 ? 0xE100 : 0;
    return static_cast<scan_code_t>(prefix | (raw.MakeCode & 0x7F));
}

/**
 * key of the raw input
 * @details by scan code, quirks of raw input:
 * - overrun or fake key of an escape sequence (`VKey == 0xFF`) -> `Key::NONE`
 * - fake Shift around `0xE0` keys (`0xE02A`, `0xE036`) -> `Key::NONE`
 * - Pause is `0xE11D 0x45` and `0x45` is NumLock: `VKey` decides `0x45`
 * - Alt + PrintScreen (`0x54`) -> `PRINT_SCREEN`, Control + Pause (`0xE046`) -> `PAUSE`
 */
[[nodiscard]] constexpr io::ex_key to_ex_key(RAWKEYBOARD const& raw) {
    using io::Key;

    if(raw.MakeCode == KEYBOARD_OVERRUN_MAKE_CODE || raw.VKey >= 0xFF)
        return {Key::NONE};

    auto const scanCode = to_scan_code(raw);
    switch(scanCode) {
        case 0x45:
            if(raw.VKey == VK_NUMLOCK)
                return {Key::NUM_LOCK};
            return {raw.VKey == VK_PAUSE ? Key::PAUSE : Key::NONE};
        case 0x54: return {Key::PRINT_SCREEN};
        case 0xE046: return {Key::PAUSE};
        case 0xE02A:
        case 0xE036: return {Key::NONE};
        default: return to_ex_key(scanCode);
    }
}

[[nodiscard]] constexpr io::key_state to_key_state(RAWKEYBOARD const& raw) {
    return io::key_state{to_ex_key(raw), (raw.Flags & RI_KEY_BREAK) == 0};
}

/**
 * output of the key in the keyboard layout of the foreground window, without modifiers
 * @details
 * - no modifiers: lowercase letters
 * - dead key -> its spacing character, for example `^`
 * - control characters stay: Enter `\r`, Tab `\t`, Backspace `\b`, Escape `\x1B`
 * - `VK_DELETE` -> Delete `\x7F` like other platforms, `ToUnicodeEx()` gives none
 * - failed UTF-8 conversion -> empty, logs a warning
 * @return UTF-8, empty -> none
 * @attention older than Windows 10 1607: changes the keyboard state, dead keys break
 */
[[nodiscard]] inline std::string to_character(RAWKEYBOARD const& raw) {
    constexpr UINT KEEP_KEYBOARD_STATE = 0x4; ///< `ToUnicodeEx()` flag, Windows 10 1607 and newer

    HKL const layout = GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), nullptr));
    std::array<BYTE, 256> const noModifiers{};
    std::array<wchar_t, 8> buffer{};

    int const result = ToUnicodeEx(
        raw.VKey,
        raw.MakeCode,
        noModifiers.data(),
        buffer.data(),
        static_cast<int>(buffer.size()),
        KEEP_KEYBOARD_STATE,
        layout
    );

    if(result == 0 && raw.VKey == VK_DELETE)
        return "\x7F";

    // -1: dead key, the buffer holds its spacing character
    std::wstring_view const units{buffer.data(), result < 0 ? 1 : static_cast<uz>(result)};

    try {
        return win::to_string(units);
    }
    catch(except::win_exception const& exception) {
        CTH_WIN_WARN(true, "failed to convert the key character to UTF-8, got {}", exception.what()) {}
        return {};
    }
}

/**
 * converts the raw input to a key update at the current time
 * @details calls `to_key_state()`, `to_character()`
 */
[[nodiscard]] inline io::key_update to_key_update(RAWKEYBOARD const& raw) {
    auto state = to_key_state(raw);
    if(state.exKey.key != io::Key::NONE)
        state.character = to_character(raw);

    return {std::move(state), chrono::clock_t::now()};
}

/**
 * converts the `state` to a keyboard `INPUT`
 * @details
 * - sends the scan code: the position, the layout of the receiver makes the character
 * - `PAUSE` -> virtual key `VK_PAUSE`, `SendInput()` has no `0xE1` flag
 * @pre `state` holds no mouse key
 */
[[nodiscard]] constexpr INPUT to_win_input(io::key_state const& state) {
    using io::Key;
    auto const key = state.exKey.key;
    CTH_CRITICAL(key >= Key::MOUSE_LEFT && key <= Key::MOUSE_5, "key must be a keyboard key, got {}", key) {}

    DWORD const up = state.down ? 0 : KEYEVENTF_KEYUP;

    if(key == Key::PAUSE)
        return {
            .type = INPUT_KEYBOARD,
            .ki{.wVk = VK_PAUSE, .wScan = 0, .dwFlags = up, .time = 0, .dwExtraInfo = 0}
        };

    auto const scanCode = to_scan_code(state.exKey);
    CTH_CRITICAL(scanCode == 0, "key must have a scan code, got {}", state.exKey) {}

    DWORD const extended = (scanCode >> 8) == 0xE0 ? KEYEVENTF_EXTENDEDKEY : 0;

    return {
        .type = INPUT_KEYBOARD,
        .ki{
            .wVk = 0,
            .wScan = static_cast<WORD>(scanCode & 0xFF),
            .dwFlags = KEYEVENTF_SCANCODE | extended | up,
            .time = 0,
            .dwExtraInfo = 0
        }
    };
}
}
