#pragma once

#include <array>
#include <iostream>
#include <locale>
#include <string>
#include <Windows.h>

#include "cth_concepts.hpp"
#include "cth_string.hpp"

namespace cth::console {
enum Ansi_Color_Ids {
    ANSI_ID_BLACK,
    ANSI_ID_DARK_RED,
    ANSI_ID_DARK_GREEN,
    ANSI_ID_DARK_YELLOW,
    ANSI_ID_DARK_BLUE,
    ANSI_ID_DARK_MAGENTA,
    ANSI_ID_DARK_CYAN,
    ANSI_ID_DARK_WHITE,
    ANSI_ID_BRIGHT_BLACK,
    ANSI_ID_BRIGHT_RED,
    ANSI_ID_BRIGHT_GREEN,
    ANSI_ID_BRIGHT_YELLOW,
    ANSI_ID_BRIGHT_BLUE,
    ANSI_ID_BRIGHT_MAGENTA,
    ANSI_ID_BRIGHT_CYAN,
    ANSI_ID_WHITE,
    ANSI_ID_CLEAR_FOREGROUND,

    ANSI_ID_BLACK_BG,
    ANSI_ID_DARK_RED_BG,
    ANSI_ID_DARK_GREEN_BG,
    ANSI_ID_DARK_YELLOW_BG,
    ANSI_ID_DARK_BLUE_BG,
    ANSI_ID_DARK_MAGENTA_BG,
    ANSI_ID_DARK_CYAN_BG,
    ANSI_ID_DARK_WHITE_BG,
    ANSI_ID_BRIGHT_BLACK_BG,
    ANSI_ID_BRIGHT_RED_BG,
    ANSI_ID_BRIGHT_GREEN_BG,
    ANSI_ID_BRIGHT_YELLOW_BG,
    ANSI_ID_BRIGHT_BLUE_BG,
    ANSI_ID_BRIGHT_MAGENTA_BG,
    ANSI_ID_BRIGHT_CYAN_BG,
    ANSI_ID_WHITE_BG,

    ANSI_ID_BOLD,
    ANSI_ID_FAINT,
    ANSI_ID_CLEAR_BOLD_FAINT,

    ANSI_ID_ITALIC,
    ANSI_ID_CLEAR_ITALIC,

    ANSI_ID_UNDERLINE,
    ANSI_ID_DOUBLE_UNDERLINE,
    ANSI_ID_CLEAR_UNDERLINE,

    ANSI_ID_BLINK,
    ANSI_ID_CLEAR_BLINK,

    ANSI_ID_INVERSE,
    ANSI_ID_CLEAR_INVERSE,

    ANSI_ID_CLEAR,

    ANSI_COLOR_IDS_SIZE
};
constexpr array<const char*, 46> ANSI_COLOR_CODES_N = {{
    //colors
    "\033[30m", "\033[31m", "\033[32m", "\033[33m",
    "\033[34m", "\033[35m", "\033[36m", "\033[37m",
    "\033[90m", "\033[91m", "\033[92m", "\033[93m",
    "\033[94m", "\033[95m", "\033[96m", "\033[97m",
    "\033[39m",

    //background colors
    "\033[40m", "\033[41m", "\033[42m", "\033[43m",
    "\033[44m", "\033[45m", "\033[46m", "\033[47m",
    "\033[100m", "\033[101m", "\033[102m", "\033[103m",
    "\033[104m", "\033[105m", "\033[106m", "\033[107m",

    //styles
    "\033[1m", "\033[2m", "\033[22m", //bold, faint, clear bold/faint
    "\033[3m", "\033[23m", //italic, clear italic
    "\033[4m", "\033[21m", "\033[24m", //underline, double underline, clear underline
    "\033[5m", "\033[25m", //blink, clear blink
    "\033[7m", "\033[27m", //inverse, clear inverse

    "\033[0m", //clear all
}};
constexpr array<const wchar_t*, 46> ANSI_COLOR_CODES_W = {{
    //colors
    L"\033[30m", L"\033[31m", L"\033[32m", L"\033[33m",
    L"\033[34m", L"\033[35m", L"\033[36m", L"\033[37m",
    L"\033[90m", L"\033[91m", L"\033[92m", L"\033[93m",
    L"\033[94m", L"\033[95m", L"\033[96m", L"\033[97m",
    L"\033[39m",

    //background colors
    L"\033[40m", L"\033[41m", L"\033[42m", L"\033[43m",
    L"\033[44m", L"\033[45m", L"\033[46m", L"\033[47m",
    L"\033[100m", L"\033[101m", L"\033[102m", L"\033[103m",
    L"\033[104m", L"\033[105m", L"\033[106m", L"\033[107m",

    //styles
    L"\033[1m", L"\033[2m", L"\033[22m", //bold, faint, clear bold/faint
    L"\033[3m", L"\033[23m", //italic, clear italic
    L"\033[4m", L"\033[21m", L"\033[24m", //underline, double underline, clear underline
    L"\033[5m", L"\033[25m", //blink, clear blink
    L"\033[7m", L"\033[27m", //inverse, clear inverse

    L"\033[0m", //clear all
}};


enum Ansi_Cursor_Ids {
    //cursor codes
    ANSI_ID_CURSOR_UP,
    ANSI_ID_CURSOR_UP_RESET_X,
    ANSI_ID_CURSOR_DOWN,
    ANSI_ID_CURSOR_DOWN_RESET_X,
    ANSI_ID_CURSOR_LEFT,
    ANSI_ID_CURSOR_RIGHT,
    ANSI_ID_CURSOR_SET_X,
    ANSI_ID_CURSOR_RESET,

    ANSI_CURSOR_IDS_SIZE
};
constexpr array<const char*, 8> ANSI_CURSOR_CODES_N{{
    //control sequences, replace the # with a number
    "\033[#A", "\033[#F", //up, up reset x
    "\033[#B", "\033[#E", //down, down reset x
    "\033[#C", "\033[#D", //left, right
    "\033[#G", //set x
    "\033[#H" //reset
}};
constexpr array<const wchar_t*, 8> ANSI_CURSOR_CODES_W{{
    //control sequences, replace the # with a number
    L"\033[#A", L"\033[#F", //up, up reset x
    L"\033[#B", L"\033[#E", //down, down reset x
    L"\033[#C", L"\033[#D", //left, right
    L"\033[#G", //set x
    L"\033[#H" //reset
}};

enum Ansi_Erase_Ids {
    //erase codes
    ANSI_ID_ERASE_CURSOR_LEFT,
    ANSI_ID_ERASE_CURSOR_RIGHT,
    ANSI_ID_ERASE_LINE,
    ANSI_ID_ERASE_SCREEN,
    ANSI_ID_ERASE_SCREEN_UP,
    ANSI_ID_ERASE_SCREEN_DOWN,
    ANSI_ERASE_IDS_SIZE
};

//TEMP clear up file
//constexpr array<const char*, 8> ANSI_ERASE_CODES_N{
//    {
//           //control sequences, replace the # with a number
//    "\033[#K", "\033[1K", "\033[2K", "\033[0K", //erase line, left, right, all
//    "\033[2J", "\033[1J", "\033[J", "\033[0J" //erase screen, up, down, all
//}}; //TEMP left off here implement the erase codes and test everything
//https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797



namespace internal {
    template<concepts::str::string_view_t T, concepts::stream::ostream_t U>
    void output(const T& str, const Ansi_Color_Ids color, const U& stream) {
        if constexpr(is_same_v<T, string_view>) stream << ANSI_COLOR_CODES_N[color] << str << ANSI_COLOR_CODES_N[ANSI_ID_CLEAR];
        else stream << ANSI_COLOR_CODES_W[color] << str << ANSI_COLOR_CODES_W[ANSI_ID_CLEAR];
    }
} // namespace cth::console::internal


#define CTH_PRINT_FUNC_TEMPLATE(name, stream) \
inline void name(const string_view str, const Ansi_Color_Ids color) { (stream) << ANSI_COLOR_CODES_N[color] << str << ANSI_COLOR_CODES_N[ANSI_ID_CLEAR]; } \
inline void name(const wstring_view str, const Ansi_Color_Ids color) { w##stream << ANSI_COLOR_CODES_W[color] << str << ANSI_COLOR_CODES_W[ANSI_ID_CLEAR]; } \
inline void name(const string_view str) { cout << str; } \
inline void name(const wstring_view str) { wcout << str; } \
inline void name##ln(const string_view str, const Ansi_Color_Ids color) { \
    (stream) << ANSI_COLOR_CODES_N[color] << str << ANSI_COLOR_CODES_N[ANSI_ID_CLEAR] << '\n'; \
} \
inline void name##ln(const wstring_view str, const Ansi_Color_Ids color) { \
    w##stream << ANSI_COLOR_CODES_W[color] << str << ANSI_COLOR_CODES_W[ANSI_ID_CLEAR] << L'\n'; \
} \
inline void name##ln(const string_view str) { (stream) << str << '\n'; } \
inline void name##ln(const wstring_view str) { w##stream << str << L'\n'; }


CTH_PRINT_FUNC_TEMPLATE(print, cout)
CTH_PRINT_FUNC_TEMPLATE(err, cerr)
} // namespace cth::console
