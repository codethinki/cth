#pragma once
#include "cth/string.hpp"
#include "cth/data/string_joiner.hpp"
#include "cth/meta/ranges.hpp"


#include <array>
#include <iostream>
#include <stack>


namespace cth::io {
using ansi_code_t = uint8_t;

class col_stream;

//-------------------------
//        CONSTANTS
//-------------------------

enum class TextColor : ansi_code_t {
    DEFAULT = 39,
    BLACK = 30,
    DARK_RED = 31,
    DARK_GREEN = 32,
    DARK_YELLOW = 33,
    DARK_BLUE = 34,
    DARK_MAGENTA = 35,
    DARK_CYAN = 36,
    DARK_WHITE = 37,
    BRIGHT_BLACK = 90,
    BRIGHT_RED = 91,
    BRIGHT_GREEN = 92,
    BRIGHT_YELLOW = 93,
    BRIGHT_BLUE = 94,
    BRIGHT_MAGENTA = 95,
    BRIGHT_CYAN = 96,
    WHITE = 97,
};

[[nodiscard]] constexpr ansi_code_t to_ansi_code(TextColor color) {
    return static_cast<ansi_code_t>(color);
}

enum class BGColor : ansi_code_t {
    DEFAULT = 40,
    BLACK = 40,
    DARK_RED = 41,
    DARK_GREEN = 42,
    DARK_YELLOW = 43,
    DARK_BLUE = 44,
    DARK_MAGENTA = 45,
    DARK_CYAN = 46,
    DARK_WHITE = 47,
    BRIGHT_BLACK = 100,
    BRIGHT_RED = 101,
    BRIGHT_GREEN = 102,
    BRIGHT_YELLOW = 103,
    BRIGHT_BLUE = 104,
    BRIGHT_MAGENTA = 105,
    BRIGHT_CYAN = 106,
    WHITE = 107,
};

[[nodiscard]] constexpr ansi_code_t to_ansi_code(BGColor color) {
    return static_cast<ansi_code_t>(color);
}



enum class TextIntensity : ansi_code_t {
    NORMAL = 22,
    BOLD = 1,
    FAINT = 2,
};

[[nodiscard]] constexpr ansi_code_t to_ansi_code(TextIntensity intensity) {
    return static_cast<ansi_code_t>(intensity);
}


enum class TextUnderline : ansi_code_t {
    NONE = 24,
    SINGLE = 4,
    DOUBLE = 21
};

[[nodiscard]] constexpr ansi_code_t to_ansi_code(TextUnderline underline) {
    return static_cast<ansi_code_t>(underline);
}

enum class TextModifiers : ansi_code_t {
    ITALIC,
    BLINK,
    INVERSE,
    HIDDEN,
    STRIKEOUT,
    TEXT_MODIFIER_SIZE
};


namespace dev {
    [[nodiscard]] constexpr ansi_code_t to_ansi_code_on(TextModifiers modifier) {
        switch(modifier) {
            case TextModifiers::ITALIC: return 3;
            case TextModifiers::BLINK: return 5;
            case TextModifiers::INVERSE: return 7;
            case TextModifiers::HIDDEN: return 8;
            case TextModifiers::STRIKEOUT: return 9;
            default: std::unreachable();
        }
    }

    [[nodiscard]] constexpr ansi_code_t to_ansi_code_off(TextModifiers modifier) {
        return to_ansi_code_on(modifier) + 20;
    }
}


[[nodiscard]] constexpr ansi_code_t to_ansi_code(TextModifiers modifier, bool activate) {
    return activate ? dev::to_ansi_code_on(modifier) : dev::to_ansi_code_off(modifier);
}



struct ansi_state {
    static constexpr size_t MODIFIERS = static_cast<size_t>(TextModifiers::TEXT_MODIFIER_SIZE);
    static constexpr size_t ATTRIBUTES = 4 + MODIFIERS;

    TextColor textColor = TextColor::DEFAULT;
    BGColor bgColor = BGColor::DEFAULT;
    TextIntensity intensity = TextIntensity::NORMAL;
    TextUnderline underline = TextUnderline::NONE;
    std::array<bool, MODIFIERS> modifiers{};

    constexpr bool& modifier(TextModifiers modifier) { return modifiers[static_cast<size_t>(modifier)]; }

    constexpr bool update(TextColor t) { return opt_update(textColor, t); }
    constexpr bool update(BGColor t) { return opt_update(bgColor, t); }
    constexpr bool update(TextIntensity t) { return opt_update(intensity, t); }
    constexpr bool update(TextUnderline t) { return opt_update(underline, t); }


    constexpr bool update(TextModifiers mod, bool on) {
        return opt_update(modifier(mod), on);
    }

    constexpr bool toggle(TextModifiers mod) { return modifier(mod) = !modifier(mod); }

    constexpr std::array<ansi_code_t, ATTRIBUTES> encode() const {
        std::array<ansi_code_t, ATTRIBUTES> codes{};

        codes[0] = to_ansi_code(textColor);
        codes[1] = to_ansi_code(bgColor);
        codes[2] = to_ansi_code(intensity);
        codes[3] = to_ansi_code(underline);

        for(size_t i = 0; i < MODIFIERS; i++)
            codes[4 + i] = to_ansi_code(static_cast<TextModifiers>(i), modifiers[i]);

        return codes;
    }

private:
    template<class T>
    constexpr static bool opt_update(T& from, T to) {
        std::swap(from, to);
        return from != to;
    }
};


namespace dev {
    template<std::ranges::viewable_range Rng1, std::ranges::viewable_range Rng2>
    [[nodiscard]] constexpr auto diff_view(Rng1& from, Rng2& to) {
        return std::views::zip(from, to)
            | std::views::filter([](auto const& pair) { return std::get<0>(pair) != std::get<1>(pair); })
            | std::views::elements<1>;
    }
}


[[nodiscard]] constexpr std::string_view ansi_clear_string() { return "\033[0m"; }

template<std::ranges::viewable_range Rng>
[[nodiscard]] std::string to_ansi_string(Rng&& codes) {
    if(std::ranges::empty(codes))
        return "";

    return std::format("\033[{}m", cth::views::format(codes, ";"));
}
[[nodiscard]] inline std::string to_ansi_string(ansi_code_t code) {
    return std::format("\033[{}m", code);
}
}


namespace cth::io::dev {

inline cxpr size_t MAX_STACK_SIZE = 16;

enum Cursor_Ids {
    // cursor codes
    ID_CURSOR_UP,
    ID_CURSOR_UP_RESET_X,
    ID_CURSOR_DOWN,
    ID_CURSOR_DOWN_RESET_X,
    ID_CURSOR_LEFT,
    ID_CURSOR_RIGHT,
    ID_CURSOR_SET_X,
    ID_CURSOR_RESET,

    CURSOR_IDS_SIZE
};
enum Erase_Ids {
    ID_ERASE_LINE,
    ID_ERASE_CURSOR_LEFT,
    ID_ERASE_CURSOR_RIGHT,
    ID_ERASE_SCREEN,
    ID_ERASE_SCREEN_UP,
    ID_ERASE_SCREEN_DOWN,
    ERASE_IDS_SIZE
};

}


namespace cth::io {

class col_stream;

struct scoped_color_state {
    constexpr scoped_color_state(col_stream& stream);
    constexpr ~scoped_color_state();

    col_stream& stream;
};

class col_stream {
public:
    constexpr col_stream(
        std::ostream& out,
        ansi_state state = {}
    ) : _out{&out},
        _stateStack{std::move(state)} {}
    constexpr ~col_stream() = default;

    constexpr void update(ansi_state new_state) {
        state() = std::move(new_state);
        invalidate_cache();
    }

    template<class T> requires (mta::convertible_to_any<T, TextColor, BGColor, TextIntensity, TextUnderline>)
    constexpr void update(T updated) {
        if(state().update(updated))
            invalidate_cache();
    }

    constexpr void update(TextModifiers modifier, bool on) {
        if(state().update(modifier, on))
            invalidate_cache();
    }

    constexpr void enable(TextModifiers modifier) { update(modifier, true); }
    constexpr void disable(TextModifiers modifier) { update(modifier, false); }

    constexpr bool toggle(TextModifiers modifier) {
        invalidate_cache();
        return state().toggle(modifier);
    }


    constexpr void pushState() {
        _stateStack.push_back(state());
    }
    constexpr void popState() {
        if(_stateStack.size() <= 1)
            return;
        _stateStack.pop_back();
        invalidate_cache();
    }
    constexpr void clearStates() {
        _stateStack[0] = state();

        _stateStack.resize(1);
        invalidate_cache();
    }

    void print(ansi_state temp_state, std::string_view str) {
        scoped_color_state _{*this};
        update(temp_state);
        print(str);
    }
    void println(ansi_state temp_state, std::string_view str) {
        print(temp_state, str);
        newline();
    }

    void print(TextColor temp_col, std::string_view str) {
        scoped_color_state _{*this};
        update(temp_col);
        print(str);
    }
    void println(TextColor temp_col, std::string_view str) {
        print(temp_col, str);
        newline();
    }

    void println(std::string_view str) {
        print(str);
        newline();
    }
    void print(std::string_view str) {
        if(str.empty())
            return;

        if(!_ansiCache)
            update_cache();
        out() << *_ansiCache << str << ansi_clear_string();
    }

private:
    void newline() const { out() << '\n'; }
    constexpr void invalidate_cache() { _ansiCache.reset(); }
    constexpr void update_cache() {
        _ansiCache = to_ansi_string(state().encode());
    }

    [[nodiscard]] constexpr ansi_state& state() { return _stateStack.back(); }

    std::ostream* _out;
    std::vector<ansi_state> _stateStack;

    std::optional<std::string> _ansiCache{};

public:
    [[nodiscard]] constexpr std::span<ansi_state const> stateStack() const { return _stateStack; }
    [[nodiscard]] constexpr ansi_state const& state() const { return _stateStack.back(); }
    [[nodiscard]] constexpr std::ostream& out() const { return *_out; }

};


}


namespace cth::io {
constexpr scoped_color_state::scoped_color_state(col_stream& stream) : stream{stream} {
    stream.pushState();
}
constexpr scoped_color_state::~scoped_color_state() { stream.popState(); }
}


namespace cth::io {

inline col_stream console{std::cout};
inline col_stream error{std::cerr};

}
