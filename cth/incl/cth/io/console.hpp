#pragma once
#include "../string.hpp"

#include <array>
#include <iostream>
#include <print>


namespace cth::io {

class col_stream;

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

    static inline cxpr size_t MAX_STACK_SIZE = 16;

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



    inline static cxpr std::array<char const*, TEXT_COL_SIZE> TEXT_COLOR_CODES_N = {{
        "\033[39m", //default
        "\033[30m", "\033[31m", "\033[32m", "\033[33m",
        "\033[34m", "\033[35m", "\033[36m", "\033[37m",
        "\033[90m", "\033[91m", "\033[92m", "\033[93m",
        "\033[94m", "\033[95m", "\033[96m", "\033[97m",
    }};
    inline static cxpr std::array<char const*, BG_COL_SIZE> BG_COLOR_CODES_N = {{
        "\033[40m", "\033[41m", "\033[42m", "\033[43m",
        "\033[44m", "\033[45m", "\033[46m", "\033[47m",
        "\033[100m", "\033[101m", "\033[102m", "\033[103m",
        "\033[104m", "\033[105m", "\033[106m", "\033[107m",
    }};
    inline static cxpr std::array<char const*, TEXT_STYLE_SIZE * 2> TEXT_STYLE_CODES_N = {{
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


    template<mta::character T = char>
    static cxpr std::string_view ansiCode(Text_Colors color) {
        return TEXT_COLOR_CODES_N[color];
    }
    template<mta::character T = char>
    static cxpr std::string_view ansiCode(BG_Colors color) {
        return BG_COLOR_CODES_N[color];
    }
    template<mta::character T = char>
    static cxpr std::string_view ansiCode(Text_Styles color) {
        return TEXT_STYLE_CODES_N[color];
    }

    //TODO add supports for these codes
    //inline static cxpr array<const char*, CURSOR_IDS_SIZE> CURSOR_CODES_N{{
    //    //control sequences, replace the # with a number
    //    "\033[#A", "\033[#F", //up, up reset x
    //    "\033[#B", "\033[#E", //down, down reset x
    //    "\033[#C", "\033[#D", //left, right
    //    "\033[#G", //set x
    //    "\033[#H" //reset
    //}};
    //inline static cxpr array<const char*, ERASE_IDS_SIZE> ERASE_CODES_N{{
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
    cxpr col_stream_state() = default;

    [[nodiscard]] cxpr Text_Colors textCol() const { return _textCol; }
    [[nodiscard]] cxpr BG_Colors bgCol() const { return _bgCol; }
    [[nodiscard]] cxpr bool styleActive(Text_Styles id) const { return _textStyles[id]; }

    cxpr void setTextCol(Text_Colors text_color) { _textCol = text_color; }
    cxpr void setBGCol(BG_Colors bg_color) { _bgCol = bg_color; }
    cxpr void resetStyle() { _textStyles.fill(false); }

    cxpr void setTextStyles(std::array<bool, TEXT_STYLE_SIZE> const& text_styles) { _textStyles = text_styles; }

    cxpr void setStyle(Text_Styles style, bool const activate = true) { _textStyles[style] = activate; }

private:
    Text_Colors _textCol = DEFAULT_TEXT_COL;
    BG_Colors _bgCol = DEFAULT_BG_COL;

    std::array<bool, TEXT_STYLE_SIZE> _textStyles{};

    friend col_stream;
};

class col_stream {
public:
    /**
     * \param out ostream to wrap
     * \param current_state for sharing state between streams\n same output streams must be shared!
     */
    explicit col_stream(std::ostream* out, std::shared_ptr<col_stream_state> const& current_state = nullptr) : _oStream(out) {
        if(current_state != nullptr) _current = current_state;
        else _current = std::make_shared<col_stream_state>();
    }


    template<bool Cache = true>
    void setTextCol(Text_Colors text_col) const;
    void setCachedTextCol() const { setTextCol<false>(_current->textCol()); }
    template<bool Cache = true>
    void setBGCol(BG_Colors bg_col) const;
    void setBGCol() const { setBGCol<false>(_current->bgCol()); }
    template<bool Cache = true>
    void setTextStyle(Text_Styles style, bool activate = true) const;

    template<bool Cache = true>
    void setState(col_stream_state new_state) const;

    template<bool Cache = true>
    void resetState() const { setState<Cache>(col_stream_state()); }

    void pushState() { _stack[_stackI++] = *_current; }
    void popState() {
        *_current = _stack[--_stackI];
        setState<false>(*_current);
    }

    [[nodiscard]] std::shared_ptr<col_stream_state> state() { return _current; }
    [[nodiscard]] std::ostream& stream() const { return *_oStream; }

    void print(std::string_view str) const { *_oStream << str; }
    void println(std::string_view str) const { *_oStream << str << '\n'; }


    void print(Text_Colors col, std::string_view str) const;
    void println(Text_Colors col, std::string_view str) const;


    template<class... Types> requires(sizeof...(Types) > 0u)
    void print(Text_Colors col, std::format_string<Types...> f_str, Types&&... types);
    template<class... Types> requires(sizeof...(Types) > 0u)
    void println(Text_Colors col, std::format_string<Types...> f_str, Types&&... types);

private:
    std::ostream* _oStream;

    std::shared_ptr<col_stream_state> _current = nullptr;
    uint32_t _stackI = 0;
    std::array<col_stream_state, dev::MAX_STACK_SIZE> _stack{};
};
} // namespace cth::io

namespace cth::io {

//TEMP left off here, the styles are bugged fix them
template<bool Cache>
void col_stream::setTextCol(Text_Colors text_col) const {
    if constexpr(Cache) _current->setTextCol(text_col);
    *_oStream << dev::ansiCode(text_col);
}
template<bool Cache>
void col_stream::setBGCol(BG_Colors bg_col) const {
    if constexpr(Cache) _current->setBGCol(bg_col);
    *_oStream << dev::ansiCode(bg_col);
}
template<bool Cache>
void col_stream::setTextStyle(Text_Styles style, bool activate) const {
    if constexpr(Cache) _current->setStyle(style, activate);
    *_oStream << dev::ansiCode(static_cast<Text_Styles>(style * 2 + (activate ? 0 : 1)));
}
template<bool Cache>
void col_stream::setState(col_stream_state new_state) const {
    if constexpr(Cache) *_current = new_state;
    setTextCol<false>(new_state.textCol());
    setBGCol<false>(new_state.bgCol());
    for(uint32_t i = 0; i < TEXT_STYLE_SIZE; i++) setTextStyle<
        false>(static_cast<Text_Styles>(i), new_state.styleActive(static_cast<Text_Styles>(i)));
}

inline void col_stream::print(Text_Colors col, std::string_view str) const {
    *_oStream << dev::ansiCode(col) << str << dev::ansiCode(_current->textCol());
}

inline void col_stream::println(Text_Colors col, std::string_view str) const {
    print(col, str);
    *_oStream << '\n';
}


template<class... Types> requires (sizeof...(Types) > 0u)
void col_stream::print(Text_Colors col, std::format_string<Types...> f_str, Types&&... types) {
    col_stream::print(col, std::format(f_str, std::forward<Types>(types)...));
}
template<class... Types> requires (sizeof...(Types) > 0u)
void col_stream::println(Text_Colors col, std::format_string<Types...> f_str, Types&&... types) {
    col_stream::println(col, std::format(f_str, std::forward<Types>(types)...));
}

} // namespace cth::io

//-------------------------
// INSTANTIATIONS
//-------------------------

namespace cth::io {}


//-------------------------
//  GLOBAL CONSOLES
//-------------------------

namespace cth::io {

inline col_stream console{&std::cout};
inline col_stream error{&std::cerr, console.state()};

}
