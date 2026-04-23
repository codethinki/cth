#pragma once
#include "cth/chrono.hpp"
#include "cth/io/log.hpp"
#include "cth/win/io/keyboard.hpp"

// private
#include "cth/win/win_include.hpp"


namespace cth::win {
struct win_key {
    WORD vk;
    DWORD flags;
};

constexpr win_key to_win_key(io::ex_key ex_key) {
    auto& [key, right] = ex_key;

    using io::Key;

    if(key >= Key::A && key <= Key::Z)
        return {static_cast<WORD>('A' + (*key - *Key::A)), 0};

    if(key >= Key::DIGIT_0 && key <= Key::DIGIT_9)
        return {static_cast<WORD>('0' + (*key - *Key::DIGIT_0)), 0};

    if(key >= Key::F1 && key <= Key::F12)
        return {static_cast<WORD>(VK_F1 + (*key - *Key::F1)), 0};

    WORD vk = 0;
    DWORD flags = 0;

    switch(key) {
        case Key::SPACE: vk = VK_SPACE;
            break;
        case Key::BACKTICK: vk = VK_OEM_3;
            break;
        case Key::MINUS: vk = VK_OEM_MINUS;
            break;
        case Key::EQUAL: vk = VK_OEM_PLUS;
            break;
        case Key::OPEN_BRACKET: vk = VK_OEM_4;
            break;
        case Key::CLOSE_BRACKET: vk = VK_OEM_6;
            break;
        case Key::BACKSLASH: vk = VK_OEM_5;
            break;
        case Key::SEMICOLON: vk = VK_OEM_1;
            break;
        case Key::QUOTE: vk = VK_OEM_7;
            break;
        case Key::COMMA: vk = VK_OEM_COMMA;
            break;
        case Key::PERIOD: vk = VK_OEM_PERIOD;
            break;
        case Key::SLASH: vk = VK_OEM_2;
            break;
        case Key::HASH: vk = VK_OEM_3;
            break;
        case Key::PLUS: vk = VK_ADD;
            break;
        case Key::STAR: vk = VK_MULTIPLY;
            break;
        case Key::LESS_THAN: vk = VK_OEM_COMMA;
            break;

        case Key::BACKSPACE: vk = VK_BACK;
            break;
        case Key::CAPS_LOCK: vk = VK_CAPITAL;
            break;
#pragma push_macro("DELETE")
#undef DELETE
        case Key::DELETE:
#pragma pop_macro("DELETE")
            vk = VK_DELETE;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::END:
            vk = VK_END;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::ENTER: vk = VK_RETURN;
            break;
        case Key::ESCAPE: vk = VK_ESCAPE;
            break;
        case Key::HOME:
            vk = VK_HOME;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::INSERT:
            vk = VK_INSERT;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::PAGE_DOWN:
            vk = VK_NEXT;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::PAGE_UP:
            vk = VK_PRIOR;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::TAB: vk = VK_TAB;
            break;

        case Key::ARROW_DOWN:
            vk = VK_DOWN;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::ARROW_LEFT:
            vk = VK_LEFT;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::ARROW_RIGHT:
            vk = VK_RIGHT;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::ARROW_UP:
            vk = VK_UP;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;

        case Key::ALT:
            vk = right ? VK_RMENU : VK_LMENU;
            if(right)
                flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::CONTROL:
            vk = right ? VK_RCONTROL : VK_LCONTROL;
            if(right)
                flags |= KEYEVENTF_EXTENDEDKEY;
            break;
        case Key::SHIFT: vk = right ? VK_RSHIFT : VK_LSHIFT;
            break;

        case Key::META: vk = VK_APPS;
            break;
        case Key::PLATFORM: vk = right ? VK_RWIN : VK_LWIN;
            break;

        case Key::NUM_LOCK:
            vk = VK_NUMLOCK;
            flags |= KEYEVENTF_EXTENDEDKEY;
            break;

        default: break;
    }

    CTH_CRITICAL(vk == 0, "failed to convert to win key") {}

    return {vk, flags};
}
}

namespace cth::win {

/**
 * Agnostic structure to hold core Windows key data
 */
struct win_key_params {
    uint32_t vkCode;
    uint32_t scanCode;
    bool isUp;
    bool isExtended;
};

/**
 * maps @ref win_key_params to a @ref cth::io::key_state
 */
[[nodiscard]] constexpr io::key_state to_key_state_generic(win_key_params const& p) {
    bool const down = !p.isUp;
    bool right = false;
    using io::Key;

    auto key = Key::NONE;
    switch(p.vkCode) {
        // --- Modifiers ---
        case VK_LSHIFT: key = Key::SHIFT;
            break;
        case VK_RSHIFT: key = Key::SHIFT;
            right = true;
            break;
        case VK_SHIFT:
            key = Key::SHIFT;
            right = (p.scanCode == 54); // RSHIFT scan code is usually 54
            break;

        case VK_LCONTROL: key = Key::CONTROL;
            break;
        case VK_RCONTROL: key = Key::CONTROL;
            right = true;
            break;
        case VK_CONTROL:
            key = Key::CONTROL;
            right = p.isExtended;
            break;

        case VK_LMENU: key = Key::ALT;
            break;
        case VK_RMENU: key = Key::ALT;
            right = true;
            break;
        case VK_MENU:
            key = Key::ALT;
            right = p.isExtended;
            break;

        case VK_LWIN: key = Key::PLATFORM;
            break;
        case VK_RWIN: key = Key::PLATFORM;
            right = true;
            break;
        case VK_APPS: key = Key::META;
            break;

        // --- System Keys ---
        case VK_SPACE: key = Key::SPACE;
            break;
        case VK_BACK: key = Key::BACKSPACE;
            break;
        case VK_CAPITAL: key = Key::CAPS_LOCK;
            break;
        case VK_RETURN: key = Key::ENTER;
            break;
        case VK_ESCAPE: key = Key::ESCAPE;
            break;
        case VK_TAB: key = Key::TAB;
            break;
        case VK_NUMLOCK: key = Key::NUM_LOCK;
            break;

#pragma push_macro("DELETE")
#undef DELETE
        case VK_DELETE: key = Key::DELETE;
            break;
#pragma pop_macro("DELETE")

        case VK_END: key = Key::END;
            break;
        case VK_HOME: key = Key::HOME;
            break;
        case VK_INSERT: key = Key::INSERT;
            break;
        case VK_NEXT: key = Key::PAGE_DOWN;
            break;
        case VK_PRIOR: key = Key::PAGE_UP;
            break;

        // --- Arrows ---
        case VK_DOWN: key = Key::ARROW_DOWN;
            break;
        case VK_LEFT: key = Key::ARROW_LEFT;
            break;
        case VK_RIGHT: key = Key::ARROW_RIGHT;
            break;
        case VK_UP: key = Key::ARROW_UP;
            break;

        // --- OEM / Symbols ---
        case VK_OEM_3: key = Key::BACKTICK;
            break;
        case VK_OEM_MINUS: key = Key::MINUS;
            break;
        case VK_OEM_PLUS: key = Key::EQUAL;
            break;
        case VK_OEM_4: key = Key::OPEN_BRACKET;
            break;
        case VK_OEM_6: key = Key::CLOSE_BRACKET;
            break;
        case VK_OEM_5: key = Key::BACKSLASH;
            break;
        case VK_OEM_1: key = Key::SEMICOLON;
            break;
        case VK_OEM_7: key = Key::QUOTE;
            break;
        case VK_OEM_COMMA: key = Key::COMMA;
            break;
        case VK_OEM_PERIOD: key = Key::PERIOD;
            break;
        case VK_OEM_2: key = Key::SLASH;
            break;
        case VK_ADD: key = Key::PLUS;
            break;
        case VK_MULTIPLY: key = Key::STAR;
            break;

        default:
            if(p.vkCode >= 'A' && p.vkCode <= 'Z')
                key = static_cast<Key>(static_cast<uint16_t>(Key::A) + (p.vkCode - 'A'));
            else if(p.vkCode >= '0' && p.vkCode <= '9')
                key = static_cast<Key>(static_cast<uint16_t>(Key::DIGIT_0) + (p.vkCode - '0'));
            break;
    }

    return io::key_state{io::ex_key{key, right}, down};
}

/**
 * ADAPTER: Low-Level Keyboard Hook (WH_KEYBOARD_LL)
 */
[[nodiscard]] constexpr io::key_state to_key_state(KBDLLHOOKSTRUCT const& s) {
    return to_key_state_generic(
        {
            s.vkCode,
            s.scanCode,
            (s.flags & LLKHF_UP) != 0,
            (s.flags & LLKHF_EXTENDED) != 0
        }
    );
}

[[nodiscard]] constexpr io::key_state to_key_state(RAWKEYBOARD const& r) {
    return to_key_state_generic(
        {
            r.VKey,
            static_cast<uint32_t>(r.MakeCode),
            (r.Flags & RI_KEY_BREAK) != 0,
            (r.Flags & RI_KEY_E0) != 0
        }
    );
}

[[nodiscard]] constexpr io::key_state to_key_state(WPARAM w_param, LPARAM l_param) {
    return to_key_state_generic(
        {
            static_cast<uint32_t>(w_param),
            static_cast<uint32_t>((l_param >> 16) & 0xFF),
            (l_param & (1 << 31)) != 0,
            // Bit 31: Transition state (0 for pressed, 1 for released)
            (l_param & (1 << 24)) != 0 // Bit 24: Extended key flag
        }
    );
}

/**
 * Timestamp wrapper: Use this to generate the final update object
 */
template<mta::convertible_to_any<RAWKEYBOARD, KBDLLHOOKSTRUCT> T>
[[nodiscard]] io::key_update to_key_update(T const& source) {
    return {
        win::to_key_state(source),
        chrono::clock_t::now()
    };
}

// Special overload for window messages since they take two params
[[nodiscard]] inline io::key_update to_key_update(WPARAM w_param, LPARAM l_param) {
    return {
        to_key_state(w_param, l_param),
        chrono::clock_t::now()
    };
}


[[nodiscard]] constexpr INPUT to_win_input(cth::io::key_state state) {
    auto const [vk, flags] = to_win_key(state.exKey);

    return {
        .type = INPUT_KEYBOARD,
        .ki{
            .wVk = vk,
            .wScan = 0,
            .dwFlags = static_cast<DWORD>(flags | (state.down ? 0 : KEYEVENTF_KEYUP)),
            .time = 0,
            .dwExtraInfo = 0
        }
    };
}
}
