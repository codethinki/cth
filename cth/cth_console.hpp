#pragma once
#include <array>
#include <codecvt>
#include <iostream>
#include <print>

#include "cth_string.hpp"


namespace cth::out {
class col_stream;

using namespace std;
//-------------------------
//        CONSTANTS
//-------------------------

enum Text_Colors {
    DEFAULT_TEXT_COL,
    BLACK_TEXT_COL,
    DARK_RED_TEXT_COL,
    DARK_GREEN_TEXT_COL,
    DARK_YELLOW_TEXT_COL,
    DARK_BLUE_TEXT_COL,
    DARK_MAGENTA_TEXT_COL,
    DARK_CYAN_TEXT_COL,
    DARK_WHITE_TEXT_COL,
    BRIGHT_BLACK_TEXT_COL,
    BRIGHT_RED_TEXT_COL,
    BRIGHT_GREEN_TEXT_COL,
    BRIGHT_YELLOW_TEXT_COL,
    BRIGHT_BLUE_TEXT_COL,
    BRIGHT_MAGENTA_TEXT_COL,
    BRIGHT_CYAN_TEXT_COL,
    WHITE_TEXT_COL,
    TEXT_COL_SIZE
};
enum BG_Colors {
    DEFAULT_BG_COL,
    BLACK_BG_COL = DEFAULT_BG_COL,
    DARK_RED_BG_COL,
    DARK_GREEN_BG_COL,
    DARK_YELLOW_BG_COL,
    DARK_BLUE_BG_COL,
    DARK_MAGENTA_BG_COL,
    DARK_CYAN_BG_COL,
    DARK_WHITE_BG_COL,
    BRIGHT_BLACK_BG_COL,
    BRIGHT_RED_BG_COL,
    BRIGHT_GREEN_BG_COL,
    BRIGHT_YELLOW_BG_COL,
    BRIGHT_BLUE_BG_COL,
    BRIGHT_MAGENTA_BG_COL,
    BRIGHT_CYAN_BG_COL,
    WHITE_BG_COL,
    BG_COL_SIZE
};
enum Text_Styles {
    BOLD_TEXT_STYLE,
    FAINT_TEXT_STYLE,
    ITALIC_TEXT_STYLE,
    UNDERLINED_TEXT_STYLE,
    DOUBLE_UNDERLINED_TEXT_STYLE,
    BLINK_TEXT_STYLE,
    INVERSE_TEXT_STYLE,
    HIDDEN_TEXT_STYLE,
    STRIKEOUT_TEXT_STYLE,
    TEXT_STYLE_SIZE
};

namespace dev {
    static inline constexpr size_t MAX_STACK_SIZE = 16;

    enum Cursor_Ids {
        //cursor codes
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



    inline static constexpr array<const char*, TEXT_COL_SIZE> TEXT_COLOR_CODES_N = {{
        "\033[39m", //default
        "\033[30m", "\033[31m", "\033[32m", "\033[33m",
        "\033[34m", "\033[35m", "\033[36m", "\033[37m",
        "\033[90m", "\033[91m", "\033[92m", "\033[93m",
        "\033[94m", "\033[95m", "\033[96m", "\033[97m",
    }};
    inline static constexpr array<const char*, BG_COL_SIZE> BG_COLOR_CODES_N = {{
        "\033[40m", "\033[41m", "\033[42m", "\033[43m",
        "\033[44m", "\033[45m", "\033[46m", "\033[47m",
        "\033[100m", "\033[101m", "\033[102m", "\033[103m",
        "\033[104m", "\033[105m", "\033[106m", "\033[107m",
    }};
    inline static constexpr array<const char*, TEXT_STYLE_SIZE * 2> TEXT_STYLE_CODES_N = {{
        "\033[1m", "\033[22m", //bold clear bold
        "\033[2m", "\033[22m", //faint, clear bold
        "\033[3m", "\033[23m", //italic, clear italic
        "\033[4m", "\033[24m", //underline, clear underline
        "\033[21m", "\033[24m", //double underline, clear underline
        "\033[5m", "\033[25m", //blink, clear blink
        "\033[7m", "\033[27m", //inverse, clear inverse
        "\033[8m", "\033[28m", //hidden, clear hidden
        "\033[9m", "\033[29m" //strikeout, clear strikeout
    }};

    inline static constexpr array<const wchar_t*, 17> TEXT_COLOR_CODES_W = {{
        L"\033[39m", //default
        L"\033[30m", L"\033[31m", L"\033[32m", L"\033[33m",
        L"\033[34m", L"\033[35m", L"\033[36m", L"\033[37m",
        L"\033[90m", L"\033[91m", L"\033[92m", L"\033[93m",
        L"\033[94m", L"\033[95m", L"\033[96m", L"\033[97m",
    }};
    inline static constexpr array<const wchar_t*, 16> BG_COLOR_CODES_W = {{
        L"\033[40m", L"\033[41m", L"\033[42m", L"\033[43m",
        L"\033[44m", L"\033[45m", L"\033[46m", L"\033[47m",
        L"\033[100m", L"\033[101m", L"\033[102m", L"\033[103m",
        L"\033[104m", L"\033[105m", L"\033[106m", L"\033[107m",
    }};
    inline static constexpr array<const wchar_t*, 18> TEXT_STYLE_CODES_W = {{
        L"\033[1m", L"\033[22m", //bold clear bold
        L"\033[2m", L"\033[22m", //faint, clear bold
        L"\033[3m", L"\033[23m", //italic, clear italic
        L"\033[4m", L"\033[24m", //underline, clear underline
        L"\033[21m", L"\033[24m", //double underline, clear underline
        L"\033[5m", L"\033[25m", //blink, clear blink
        L"\033[7m", L"\033[27m", //inverse, clear inverse
        L"\033[8m", L"\033[28m", //hidden, clear hidden
        L"\033[9m", L"\033[29m" //strikeout, clear strikeout
    }};

    template<type::char_t T = char>
    static constexpr basic_string_view<T> ansiCode(const Text_Colors color) {
        if constexpr(type::is_nchar_v<T>) return TEXT_COLOR_CODES_N[color];
        else return TEXT_COLOR_CODES_W[color];
    }
    template<type::char_t T = char>
    static constexpr basic_string_view<T> ansiCode(const BG_Colors color) {
        if constexpr(is_same_v<char, remove_cv_t<T>>) return BG_COLOR_CODES_N[color];
        else return BG_COLOR_CODES_W[color];
    }
    template<type::char_t T = char>
    static constexpr basic_string_view<T> ansiCode(const Text_Styles color) {
        if constexpr(is_same_v<char, remove_cv_t<T>>) return TEXT_STYLE_CODES_N[color];
        else return TEXT_STYLE_CODES_W[color];
    }

    //TODO add supports for these codes
    //inline static constexpr array<const char*, CURSOR_IDS_SIZE> CURSOR_CODES_N{{
    //    //control sequences, replace the # with a number
    //    "\033[#A", "\033[#F", //up, up reset x
    //    "\033[#B", "\033[#E", //down, down reset x
    //    "\033[#C", "\033[#D", //left, right
    //    "\033[#G", //set x
    //    "\033[#H" //reset
    //}};
    //inline static constexpr array<const char*, ERASE_IDS_SIZE> ERASE_CODES_N{{
    //    "\033[2K", //erase line
    //    "\033[1K", "\033[0K", //erase line left, right
    //    "\033[2J", //erase screen
    //    "\033[1J", "\033[0J" //erase screen up, down
    //}};

} // namespace dev

//-------------------------
//    DECLARATIONS
//-------------------------


struct col_stream_state {
    constexpr col_stream_state() = default;

    [[nodiscard]] constexpr Text_Colors textCol() const { return textCol_; }
    [[nodiscard]] constexpr BG_Colors bgCol() const { return bgCol_; }
    [[nodiscard]] constexpr bool styleActive(const Text_Styles id) const { return textStyles[id]; }

    constexpr void setTextCol(const Text_Colors text_color) { textCol_ = text_color; }
    constexpr void setBGCol(const BG_Colors bg_color) { bgCol_ = bg_color; }
    constexpr void resetStyle() { textStyles.fill(0); }

    constexpr void setTextStyles(const array<bool, TEXT_STYLE_SIZE>& text_styles) { textStyles = text_styles; }

    constexpr void setStyle(const Text_Styles style, const bool activate = true) { textStyles[style] = activate; }

private:
    Text_Colors textCol_ = DEFAULT_TEXT_COL;
    BG_Colors bgCol_ = DEFAULT_BG_COL;

    array<bool, TEXT_STYLE_SIZE> textStyles{};

    friend col_stream;
};

class col_stream {
public:
    /**
     * \param out ostream to wrap
     * \param current_state for sharing state between streams\n same output streams must be shared!
     */
    explicit col_stream(ostream* out, const shared_ptr<col_stream_state>& current_state = nullptr) : oStream(out) {
        if(current_state != nullptr) current = current_state;
        else current = make_shared<col_stream_state>();
    }


    template<bool Cache = true>
    void setTextCol(Text_Colors text_col) const;
    void setCachedTextCol() const { setTextCol<false>(current->textCol()); }
    template<bool Cache = true>
    void setBGCol(BG_Colors bg_col) const;
    void setBGCol() const { setBGCol<false>(current->bgCol()); }
    template<bool Cache = true>
    void setTextStyle(Text_Styles style, bool activate = true) const;

    template<bool Cache = true>
    void setState(col_stream_state new_state) const;

    template<bool Cache = true>
    void resetState() const { setState<Cache>(col_stream_state()); }

    void pushState() { stack[stackI++] = *current; }
    void popState() {
        *current = stack[--stackI];
        setState<false>(*current);
    }

    [[nodiscard]] shared_ptr<col_stream_state> state() { return current; }
    [[nodiscard]] ostream& stream() const { return *oStream; }

    void print(const string_view str) const { *oStream << str; }
    void println(const string_view str) const { *oStream << str << '\n'; }
    void print(const wstring_view str) const { *oStream << str::conv::narrow(str.data()); }
    void println(const wstring_view str) const { *oStream << str::conv::narrow(str.data()) << '\n'; }


    void print(Text_Colors col, string_view str) const;
    void println(Text_Colors col, string_view str) const;
    void print(const Text_Colors col, const wstring_view str) const { print(col, str::conv::narrow(str)); }
    void println(const Text_Colors col, const wstring_view str) const { println(col, str::conv::narrow(str)); }


    template<typename... Types> enable_if_t<(sizeof...(Types) > 0u), void>
    print(Text_Colors col, format_string<Types...> f_str, Types&&... types);
    template<typename... Types> enable_if_t<(sizeof...(Types) > 0u), void>
    println(Text_Colors col, format_string<Types...> f_str, Types&&... types);

private:
    ostream* oStream;

    shared_ptr<col_stream_state> current = nullptr;
    uint32_t stackI = 0;
    array<col_stream_state, dev::MAX_STACK_SIZE> stack{};
};
} // namespace cth::out

namespace cth::out {
using namespace std;
//TEMP left off here, the styles are bugged fix them
template<bool Cache>
void col_stream::setTextCol(const Text_Colors text_col) const {
    if constexpr(Cache) current->setTextCol(text_col);
    *oStream << dev::ansiCode(text_col);
}
template<bool Cache>
void col_stream::setBGCol(const BG_Colors bg_col) const {
    if constexpr(Cache) current->setBGCol(bg_col);
    *oStream << dev::ansiCode(bg_col);
}
template<bool Cache>
void col_stream::setTextStyle(const Text_Styles style, const bool activate) const {
    if constexpr(Cache) current->setStyle(style, activate);
    *oStream << dev::ansiCode(static_cast<Text_Styles>(style * 2 + (activate ? 0 : 1)));
}
template<bool Cache>
void col_stream::setState(const col_stream_state new_state) const {
    if constexpr(Cache) *current = new_state;
    setTextCol<false>(new_state.textCol());
    setBGCol<false>(new_state.bgCol());
    for(uint32_t i = 0; i < TEXT_STYLE_SIZE; i++) setTextStyle<false>((Text_Styles) i, new_state.styleActive((Text_Styles) i));
}

inline void col_stream::print(const Text_Colors col, const string_view str) const {
    *oStream << dev::ansiCode(col) << str << dev::ansiCode(current->textCol());
}

inline void col_stream::println(const Text_Colors col, const string_view str) const {
    print(col, str);
    *oStream << '\n';
}


template<typename... Types> enable_if_t<(sizeof...(Types) > 0u), void>
col_stream::print(const Text_Colors col, const format_string<Types...> f_str, Types&&... types) {
    col_stream::print(col, std::format(f_str, std::forward<Types>(types)...));
}
template<typename... Types> enable_if_t<(sizeof...(Types) > 0u), void>
col_stream::println(const Text_Colors col, const format_string<Types...> f_str, Types&&... types) {
    col_stream::println(col, std::format(f_str, std::forward<Types>(types)...));
}

} // namespace cth::out

//-------------------------
// INSTANTIATIONS
//-------------------------

namespace cth::out {}

//-------------------------
//  GLOBAL CONSOLES
//-------------------------

namespace cth::out {

inline col_stream console{&std::cout};
inline col_stream error{&std::cerr, console.state()};

}
