#include "cth/win/io/keyboard.hpp"

#include "../win_include.hpp"

namespace cth::win {
namespace {

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
            case Key::SPACE: vk = VK_SPACE; break;
            case Key::BACKTICK: vk = VK_OEM_3; break;
            case Key::MINUS: vk = VK_OEM_MINUS; break;
            case Key::EQUAL: vk = VK_OEM_PLUS; break;
            case Key::OPEN_BRACKET: vk = VK_OEM_4; break;
            case Key::CLOSE_BRACKET: vk = VK_OEM_6; break;
            case Key::BACKSLASH: vk = VK_OEM_5; break;
            case Key::SEMICOLON: vk = VK_OEM_1; break;
            case Key::QUOTE: vk = VK_OEM_7; break;
            case Key::COMMA: vk = VK_OEM_COMMA; break;
            case Key::PERIOD: vk = VK_OEM_PERIOD; break;
            case Key::SLASH: vk = VK_OEM_2; break;
            case Key::HASH: vk = VK_OEM_3; break;
            case Key::PLUS: vk = VK_ADD; break;
            case Key::STAR: vk = VK_MULTIPLY; break;
            case Key::LESS_THAN: vk = VK_OEM_COMMA; break;

            case Key::BACKSPACE: vk = VK_BACK; break;
            case Key::CAPS_LOCK: vk = VK_CAPITAL; break;
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
            case Key::ENTER: vk = VK_RETURN; break;
            case Key::ESCAPE: vk = VK_ESCAPE; break;
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
            case Key::TAB: vk = VK_TAB; break;

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
            case Key::SHIFT: vk = right ? VK_RSHIFT : VK_LSHIFT; break;

            case Key::META: vk = VK_APPS; break;
            case Key::PLATFORM: vk = right ? VK_RWIN : VK_LWIN; break;

            case Key::NUM_LOCK:
                vk = VK_NUMLOCK;
                flags |= KEYEVENTF_EXTENDEDKEY;
                break;

            default: break;
        }

        CTH_CRITICAL(vk == 0, "failed to convert to win key") {}

        return {vk, flags};
    }


    constexpr INPUT to_win_input(io::key_state state) {
        auto const [vk, flags] = to_win_key(state.exKey);

        return {
            .type = INPUT_KEYBOARD,
            .ki{.wVk = vk,
                .wScan = 0,
                .dwFlags = static_cast<DWORD>(flags | (state.down ? 0 : KEYEVENTF_KEYUP)),
                .time = 0,
                .dwExtraInfo = 0}
        };
    }
}

}


namespace cth::win {


void keybd::send(io::key_state state) {
    auto input = to_win_input(state);

    SendInput(1, &input, sizeof(INPUT));
}
void keybd::send(std::span<io::key_state const> states) {
    std::vector inputs{std::from_range, states | std::views::transform([](auto const& state) {
                                            return win::to_win_input(state);
                                        })};

    SendInput(states.size(), inputs.data(), sizeof(INPUT));
}

}
