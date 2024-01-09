#pragma once

#include <array>
#include <iostream>
#include <locale>
#include <stack>
#include <string>
#include <Windows.h>

#include "cth_concepts.hpp"
#include "cth_string.hpp"

namespace cth::console {
using console_state_t = uint32_t;

//-----------------------------
//VARIABLES / CONSTANTS
//-----------------------------

enum Text_Col_Ids {
    TEXT_COL_IDS_FIRST = 0,
    COL_ID_DEFAULT_TEXT = TEXT_COL_IDS_FIRST,

    COL_ID_BLACK_TEXT,
    COL_ID_DARK_RED_TEXT,
    COL_ID_DARK_GREEN_TEXT,
    COL_ID_DARK_YELLOW_TEXT,
    COL_ID_DARK_BLUE_TEXT,
    COL_ID_DARK_MAGENTA_TEXT,
    COL_ID_DARK_CYAN_TEXT,
    COL_ID_DARK_WHITE_TEXT,
    COL_ID_BRIGHT_BLACK_TEXT,
    COL_ID_BRIGHT_RED_TEXT,
    COL_ID_BRIGHT_GREEN_TEXT,
    COL_ID_BRIGHT_YELLOW_TEXT,
    COL_ID_BRIGHT_BLUE_TEXT,
    COL_ID_BRIGHT_MAGENTA_TEXT,
    COL_ID_BRIGHT_CYAN_TEXT,
    COL_ID_WHITE_TEXT,
};
enum Bg_Col_Ids {
    BG_COL_IDS_FIRST = 0xff + 1,

    COL_ID_BLACK_BG = BG_COL_IDS_FIRST,
    COL_ID_DARK_RED_BG,
    COL_ID_DARK_GREEN_BG,
    COL_ID_DARK_YELLOW_BG,
    COL_ID_DARK_BLUE_BG,
    COL_ID_DARK_MAGENTA_BG,
    COL_ID_DARK_CYAN_BG,
    COL_ID_DARK_WHITE_BG,
    COL_ID_BRIGHT_BLACK_BG,
    COL_ID_BRIGHT_RED_BG,
    COL_ID_BRIGHT_GREEN_BG,
    COL_ID_BRIGHT_YELLOW_BG,
    COL_ID_BRIGHT_BLUE_BG,
    COL_ID_BRIGHT_MAGENTA_BG,
    COL_ID_BRIGHT_CYAN_BG,
    COL_ID_WHITE_BG,
};
enum Text_Style_Ids {
    TEXT_STYLE_IDS_FIRST = 0xffff + 1,

    STYLE_ID_NO_BOLD_TEXT,
    STYLE_ID_NO_FAINT_TEXT = STYLE_ID_NO_BOLD_TEXT,
    STYLE_ID_BOLD_TEXT,
    STYLE_ID_FAINT_TEXT,

    STYLE_ID_ITALIC_TEXT,
    STYLE_ID_NO_ITALIC_TEXT,

    STYLE_ID_NO_UNDERLINED_TEXT,
    STYLE_ID_UNDERLINED_TEXT,
    STYLE_ID_DOUBLE_UNDERLINED_TEXT,

    STYLE_ID_BLINK_TEXT,
    STYLE_ID_NO_BLINK_TEXT,

    STYLE_ID_INVERSE_TEXT,
    STYLE_ID_NO_INVERSE_TEXT,
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
inline constexpr array<const char*, 12> TEXT_STYLE_CODES = {{
    "\033[22m", "\033[1m", "\033[2m", //clear bold/faint, bold, faint,
    "\033[3m", "\033[23m", //italic, clear italic
    "\033[24m", "\033[4m", "\033[21m", //clear underline, underline, double underline
    "\033[5m", "\033[25m", //blink, clear blink
    "\033[7m", "\033[27m", //inverse, clear inverse
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

namespace internal {
    [[nodiscard]] console_state_t initialState();
    console_state_t currentState = initialState();

    constexpr uint32_t MAX_STACK_SIZE = 16;
    uint32_t stackIndex = 0;
    array<console_state_t, MAX_STACK_SIZE> stack{};
}

//-------------------------
//     DECLARATIONS
//-------------------------

template<bool CacheToState = true>
void setTextCol(Text_Col_Ids text_col);

template<bool CacheToState = true>
void setBgCol(Bg_Col_Ids bg_col);

template<bool CacheToState = true>
void setTextStyle(Text_Style_Ids text_style);

template<bool Cache = true>
void setState(console_state_t new_state);

//-----------------------------
//   ONE LINE FUNCTIONS
//-----------------------------

[[nodiscard]] inline console_state_t setTextCol(const Text_Col_Ids text_color, const console_state_t state) {
    return (state & 0xffffff00) | text_color;
}
[[nodiscard]] inline console_state_t setBgCol(const Bg_Col_Ids bg_color, const console_state_t state) { return (state & 0xffff00ff) | bg_color; }
[[nodiscard]] inline console_state_t setTextStyle(const Text_Style_Ids text_style, const console_state_t state) {
    return (state & 0xff00ffff) | text_style;
}

inline void cacheTextCol(const Text_Col_Ids text_col) { internal::currentState = setTextCol(text_col, internal::currentState); }
inline void cacheBgCol(const Bg_Col_Ids bg_col) { internal::currentState = setBgCol(bg_col, internal::currentState); }
inline void cacheTextStyle(const Text_Style_Ids text_style) { internal::currentState = setTextStyle(text_style, internal::currentState); }


[[nodiscard]] inline Text_Col_Ids toTextColorId(const console_state_t console_state) { return static_cast<Text_Col_Ids>(console_state & 0xff); }
[[nodiscard]] inline Bg_Col_Ids toBgColorId(const console_state_t console_state) { return static_cast<Bg_Col_Ids>(console_state & 0xff00); }
[[nodiscard]] inline Text_Style_Ids toTextStyleId(const console_state_t console_state) {
    return static_cast<Text_Style_Ids>(console_state & 0xff0000);
}

inline void pushState() { internal::stack[internal::stackIndex++] = internal::currentState; }
inline void popState() { setState(internal::stack[--internal::stackIndex]); }

[[nodiscard]] inline Text_Col_Ids getTextColorId() { return toTextColorId(internal::currentState); }
[[nodiscard]] inline Bg_Col_Ids getBgColorId() { return toBgColorId(internal::currentState); }
[[nodiscard]] inline Text_Style_Ids getTextStyleId() { return toTextStyleId(internal::currentState); }

//-------------------------
//       DEFINITIONS
//-------------------------
template<bool CacheToState>
void setTextCol(const Text_Col_Ids text_col) {
    if constexpr(CacheToState) internal::currentState = (internal::currentState & 0xffffff00) | text_col;
    cout << TEXT_COLOR_CODES[text_col - TEXT_COL_IDS_FIRST];
}

template<bool CacheToState>

void setBgCol(const Bg_Col_Ids bg_col) {
    if constexpr(CacheToState) internal::currentState = (internal::currentState & 0xffff00ff) | bg_col;
    cout << BG_COLOR_CODES[bg_col - BG_COL_IDS_FIRST];
}
template<bool CacheToState>
void setTextStyle(const Text_Style_Ids text_style) {
    if constexpr(CacheToState) internal::currentState = (internal::currentState & 0xff00ffff) | text_style;
    cout << TEXT_STYLE_CODES[text_style - TEXT_STYLE_IDS_FIRST];
}

template<bool Cache>
void setState(const console_state_t new_state) {
    if constexpr(Cache) internal::currentState = new_state;
    setTextCol<false>(toTextColorId(internal::currentState));
    setBgCol<false>(toBgColorId(internal::currentState));
    setTextStyle<false>(toTextStyleId(internal::currentState));
}

#define CTH_PRINT_FUNC_TEMPLATE(name, stream) \
inline void name(const string_view str, const Text_Col_Ids color) { (stream) << TEXT_COLOR_CODES[color] << str << TEXT_COLOR_CODES[getTextColorId()]; } \
inline void name(const wstring_view str, const Text_Col_Ids color) { \
    (stream) << TEXT_COLOR_CODES[color]; \
    w##stream << str; \
    (stream) << TEXT_COLOR_CODES[getTextColorId()]; \
    } \
inline void name(const string_view str) { cout << str; } \
inline void name(const wstring_view str) { wcout << str; } \
inline void name##ln(const string_view str, const Text_Col_Ids color) { \
    (stream) << TEXT_COLOR_CODES[color] << str << TEXT_COLOR_CODES[getTextColorId()] << '\n'; \
} \
inline void name##ln(const wstring_view str, const Text_Col_Ids color) { \
     name(str, color); \
     w##stream << L'\n'; \
} \
inline void name##ln(const string_view str) { (stream) << str << '\n'; } \
inline void name##ln(const wstring_view str) { w##stream << str << L'\n'; }

CTH_PRINT_FUNC_TEMPLATE(print, cout)
CTH_PRINT_FUNC_TEMPLATE(err, cerr)


namespace internal {
    inline console_state_t initialState() {
        constexpr console_state_t x = static_cast<int>(TEXT_COL_IDS_FIRST) | static_cast<int>(BG_COL_IDS_FIRST) |
            static_cast<int>(TEXT_STYLE_IDS_FIRST);
        setState(x);
        return x;
    }
} //namespace internal
} // namespace cth::console
