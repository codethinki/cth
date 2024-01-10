#pragma once

#include <array>
#include <iostream>
#include <locale>
#include <stack>
#include <string>

#include "cth_concepts.hpp"
#include "cth_string.hpp"

namespace cth::console {
using namespace std;

/**
 * \brief type to store the console state\n its split up into 4 8 bit parts (high to low: nothing, text style, text col, BG col)
 */
using state_t = uint32_t;

namespace internal {
    inline constexpr int TEXT_COL_IDS_SHIFT = 0;
    inline constexpr int BG_COL_IDS_SHIFT = 8;
    inline constexpr int TEXT_STYLE_IDS_SHIFT = 16;
}

//-----------------------------
//VARIABLES / CONSTANTS
//-----------------------------

enum Text_Col_Ids {
    COL_ID_DEFAULT_TEXT,

    COL_ID_BLACK_TEXT = 1 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_DARK_RED_TEXT = 2 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_DARK_GREEN_TEXT = 3 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_DARK_YELLOW_TEXT = 4 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_DARK_BLUE_TEXT = 5 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_DARK_MAGENTA_TEXT = 6 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_DARK_CYAN_TEXT = 7 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_DARK_WHITE_TEXT = 8 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_BRIGHT_BLACK_TEXT = 9 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_BRIGHT_RED_TEXT = 10 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_BRIGHT_GREEN_TEXT = 11 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_BRIGHT_YELLOW_TEXT = 12 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_BRIGHT_BLUE_TEXT = 13 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_BRIGHT_MAGENTA_TEXT = 14 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_BRIGHT_CYAN_TEXT = 15 << internal::TEXT_COL_IDS_SHIFT,
    COL_ID_WHITE_TEXT = 16 << internal::TEXT_COL_IDS_SHIFT,
};
enum BG_Col_Ids {
    COL_ID_DEFAULT_BG,

    COL_ID_BLACK_BG = COL_ID_DEFAULT_BG,
    COL_ID_DARK_RED_BG = 1 << internal::BG_COL_IDS_SHIFT,
    COL_ID_DARK_GREEN_BG = 2 << internal::BG_COL_IDS_SHIFT,
    COL_ID_DARK_YELLOW_BG = 3 << internal::BG_COL_IDS_SHIFT,
    COL_ID_DARK_BLUE_BG = 4 << internal::BG_COL_IDS_SHIFT,
    COL_ID_DARK_MAGENTA_BG = 5 << internal::BG_COL_IDS_SHIFT,
    COL_ID_DARK_CYAN_BG = 6 << internal::BG_COL_IDS_SHIFT,
    COL_ID_DARK_WHITE_BG = 7 << internal::BG_COL_IDS_SHIFT,
    COL_ID_BRIGHT_BLACK_BG = 8 << internal::BG_COL_IDS_SHIFT,
    COL_ID_BRIGHT_RED_BG = 9 << internal::BG_COL_IDS_SHIFT,
    COL_ID_BRIGHT_GREEN_BG = 10 << internal::BG_COL_IDS_SHIFT,
    COL_ID_BRIGHT_YELLOW_BG = 11 << internal::BG_COL_IDS_SHIFT,
    COL_ID_BRIGHT_BLUE_BG = 12 << internal::BG_COL_IDS_SHIFT,
    COL_ID_BRIGHT_MAGENTA_BG = 13 << internal::BG_COL_IDS_SHIFT,
    COL_ID_BRIGHT_CYAN_BG = 14 << internal::BG_COL_IDS_SHIFT,
    COL_ID_WHITE_BG = 15 << internal::BG_COL_IDS_SHIFT,
};
enum Text_Style_Ids {
    STYLE_ID_DEFAULT_TEXT,

    STYLE_ID_BOLD_TEXT = 1 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_FAINT_TEXT = 2 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_NO_BOLD_TEXT = 3 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_NO_FAINT_TEXT = STYLE_ID_NO_BOLD_TEXT,

    STYLE_ID_ITALIC_TEXT = 4 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_NO_ITALIC_TEXT = 5 << internal::TEXT_STYLE_IDS_SHIFT,

    STYLE_ID_UNDERLINED_TEXT = 6 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_DOUBLE_UNDERLINED_TEXT = 7 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_NO_UNDERLINED_TEXT = 8 << internal::TEXT_STYLE_IDS_SHIFT,

    STYLE_ID_BLINK_TEXT = 9 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_NO_BLINK_TEXT = 10 << internal::TEXT_STYLE_IDS_SHIFT,

    STYLE_ID_INVERSE_TEXT = 11 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_NO_INVERSE_TEXT = 12 << internal::TEXT_STYLE_IDS_SHIFT,

    STYLE_ID_HIDDEN_TEXT = 13 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_NO_HIDDEN_TEXT = 14 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_STRIKEOUT_TEXT = 15 << internal::TEXT_STYLE_IDS_SHIFT,
    STYLE_ID_NO_STRIKEOUT_TEXT = 16 << internal::TEXT_STYLE_IDS_SHIFT,
};

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

inline constexpr array<const char*, 17> TEXT_COLOR_CODES = {{
    "\033[39m", //default
    "\033[30m", "\033[31m", "\033[32m", "\033[33m",
    "\033[34m", "\033[35m", "\033[36m", "\033[37m",
    "\033[90m", "\033[91m", "\033[92m", "\033[93m",
    "\033[94m", "\033[95m", "\033[96m", "\033[97m",
}};
inline constexpr array<const char*, 16> BG_COLOR_CODES = {{
    "\033[40m", "\033[41m", "\033[42m", "\033[43m",
    "\033[44m", "\033[45m", "\033[46m", "\033[47m",
    "\033[100m", "\033[101m", "\033[102m", "\033[103m",
    "\033[104m", "\033[105m", "\033[106m", "\033[107m",
}};
inline constexpr array<const char*, 17> TEXT_STYLE_CODES = {{
    "\033[22;23;24;25;27;28;29m", //default
    "\033[1m", "\033[2m", "\033[22m", //bold, faint, clear bold/faint
    "\033[3m", "\033[23m", //italic, clear italic
    "\033[4m", "\033[21m", "\033[24m", //underline, double underline, clear underline
    "\033[5m", "\033[25m", //blink, clear blink
    "\033[7m", "\033[27m", //inverse, clear inverse
    "\033[8m", "\033[28m", //hidden, clear hidden
    "\033[9m", "\033[29m" //strikeout, clear strikeout
}};

//TODO add supports for these codes
constexpr array<const char*, CURSOR_IDS_SIZE> CURSOR_CODES_N{{
    //control sequences, replace the # with a number
    "\033[#A", "\033[#F", //up, up reset x
    "\033[#B", "\033[#E", //down, down reset x
    "\033[#C", "\033[#D", //left, right
    "\033[#G", //set x
    "\033[#H" //reset
}};
constexpr array<const char*, ERASE_IDS_SIZE> ERASE_CODES_N{{
    "\033[2K", //erase line
    "\033[1K", "\033[0K", //erase line left, right
    "\033[2J", //erase screen
    "\033[1J", "\033[0J" //erase screen up, down
}};

namespace state {
    namespace internal {
        [[nodiscard]] state_t initialState();

        constexpr uint32_t MAX_STACK_SIZE = 16;
        uint32_t stackIndex = 0;
        array<state_t, MAX_STACK_SIZE> stack{};
    } // namespace internal

    state_t current = internal::initialState(); //represents the current cached state of the console


} // namespace state



//-------------------------
//     DECLARATIONS
//-------------------------

template<bool CacheToState = true>
void setTextCol(Text_Col_Ids text_col);
template<bool CacheToState = true>
void setBGCol(BG_Col_Ids bg_col);
template<bool CacheToState = true>
void setTextStyle(Text_Style_Ids text_style);

namespace state {
    template<bool Cache = true>
    void set(state_t new_state);
}


//-----------------------------
//   ONE LINE FUNCTIONS
//-----------------------------



namespace state {
    inline void push() { internal::stack[internal::stackIndex++] = current; }
    inline void pop() { set(internal::stack[--internal::stackIndex]); }

    [[nodiscard]] inline Text_Col_Ids toTextColorId(const state_t console_state) { return static_cast<Text_Col_Ids>(console_state & 0xff); }
    [[nodiscard]] inline BG_Col_Ids toBGColorId(const state_t console_state) { return static_cast<BG_Col_Ids>(console_state & 0xff00); }
    [[nodiscard]] inline Text_Style_Ids toTextStyleId(const state_t console_state) { return static_cast<Text_Style_Ids>(console_state & 0xff0000); }

    [[nodiscard]] inline state_t setTextCol(const Text_Col_Ids text_color, const state_t state) { return (state & 0xffffff00) | text_color; }
    [[nodiscard]] inline state_t setBGCol(const BG_Col_Ids bg_color, const state_t state) { return (state & 0xffff00ff) | bg_color; }
    [[nodiscard]] inline state_t setTextStyle(const Text_Style_Ids text_style, const state_t state) { return (state & 0xff00ffff) | text_style; }

    inline void cacheTextCol(const Text_Col_Ids text_col) { current = setTextCol(text_col, current); }
    inline void cacheBGCol(const BG_Col_Ids bg_col) { current = setBGCol(bg_col, current); }
    inline void cacheTextStyle(const Text_Style_Ids text_style) { current = setTextStyle(text_style, current); }
} // namespace state

[[nodiscard]] inline Text_Col_Ids getTextColorId() { return state::toTextColorId(state::current); }
[[nodiscard]] inline BG_Col_Ids getBGColorId() { return state::toBGColorId(state::current); }
[[nodiscard]] inline Text_Style_Ids getTextStyleId() { return state::toTextStyleId(state::current); }

[[nodiscard]] constexpr size_t toArrayIndex(const Text_Col_Ids text_col) { return (text_col >> internal::TEXT_COL_IDS_SHIFT) & 0xff; }
[[nodiscard]] constexpr size_t toArrayIndex(const BG_Col_Ids bg_col) { return (bg_col >> internal::BG_COL_IDS_SHIFT) & 0xff; }
[[nodiscard]] constexpr size_t toArrayIndex(const Text_Style_Ids text_style) { return (text_style >> internal::TEXT_STYLE_IDS_SHIFT) & 0xff; }
//-------------------------
//       DEFINITIONS
//-------------------------
template<bool CacheToState>
void setTextCol(const Text_Col_Ids text_col) {
    if constexpr(CacheToState) state::cacheTextCol(text_col);
    cout << TEXT_COLOR_CODES[toArrayIndex(text_col)];
}

template<bool CacheToState>

void setBGCol(const BG_Col_Ids bg_col) {
    if constexpr(CacheToState) state::cacheBGCol(bg_col);
    cout << BG_COLOR_CODES[toArrayIndex(bg_col)];
}
template<bool CacheToState>
void setTextStyle(const Text_Style_Ids text_style) {
    if constexpr(CacheToState) state::cacheTextStyle(text_style);
    cout << TEXT_STYLE_CODES[toArrayIndex(text_style)];
}

namespace state {
    template<bool Cache>
    void set(const state_t new_state) {
        if constexpr(Cache) state::current = new_state;
        setTextCol<false>(toTextColorId(new_state));
        setBGCol<false>(toBGColorId(new_state));
        setTextStyle<false>(toTextStyleId(new_state));
    }

}

#define CTH_PRINT_FUNC_TEMPLATE(name, stream) \
inline void name(const string_view str, const Text_Col_Ids color) { (stream) << TEXT_COLOR_CODES[toArrayIndex(color)] << str << TEXT_COLOR_CODES[getTextColorId()]; } \
inline void name(const wstring_view str, const Text_Col_Ids color) { \
    (stream) << TEXT_COLOR_CODES[toArrayIndex(color)]; \
    w##stream << str; \
    (stream) << TEXT_COLOR_CODES[getTextColorId()]; \
    } \
inline void name(const string_view str) { cout << str; } \
inline void name(const wstring_view str) { wcout << str; } \
inline void name##ln(const string_view str, const Text_Col_Ids color) { \
    (stream) << TEXT_COLOR_CODES[toArrayIndex(color)] << str << TEXT_COLOR_CODES[getTextColorId()] << '\n'; \
} \
inline void name##ln(const wstring_view str, const Text_Col_Ids color) { \
     name(str, color); \
     w##stream << L'\n'; \
} \
inline void name##ln(const string_view str) { (stream) << str << '\n'; } \
inline void name##ln(const wstring_view str) { w##stream << str << L'\n'; }

CTH_PRINT_FUNC_TEMPLATE(print, cout)
CTH_PRINT_FUNC_TEMPLATE(err, cerr)


namespace state::internal {
    inline state_t initialState() {
        constexpr state_t x = static_cast<int>(COL_ID_DEFAULT_TEXT) | static_cast<int>(COL_ID_DEFAULT_BG) |
            static_cast<int>(STYLE_ID_DEFAULT_TEXT);
        set(x);
        return x;
    }
} //namespace internal
} // namespace cth::console
