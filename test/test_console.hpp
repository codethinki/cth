#pragma once
#include <gtest/gtest.h>
#include "../cth/src/cth_console.hpp"

namespace cth {


TEST(headerConsole, textColor) {
    using namespace cth::console;

    println("i am black", COL_ID_BLACK_TEXT);
    println("i am dark red", COL_ID_DARK_RED_TEXT);
    println("i am dark green", COL_ID_DARK_GREEN_TEXT);
    println("i am dark yellow", COL_ID_DARK_YELLOW_TEXT);
    println("i am dark blue", COL_ID_DARK_BLUE_TEXT);
    println("i am dark magenta", COL_ID_DARK_MAGENTA_TEXT);
    println("i am dark cyan", COL_ID_DARK_CYAN_TEXT);
    println("i am dark white", COL_ID_DARK_WHITE_TEXT);
    println("i am bright black", COL_ID_BRIGHT_BLACK_TEXT);
    println("i am bright red", COL_ID_BRIGHT_RED_TEXT);
    println("i am bright green", COL_ID_BRIGHT_GREEN_TEXT);
    println("i am bright yellow", COL_ID_BRIGHT_YELLOW_TEXT);
    println("i am bright blue", COL_ID_BRIGHT_BLUE_TEXT);
    println("i am bright magenta", COL_ID_BRIGHT_MAGENTA_TEXT);
    println("i am bright cyan", COL_ID_BRIGHT_CYAN_TEXT);
    println("i am white", COL_ID_WHITE_TEXT);

    println("i am default", COL_ID_DEFAULT_TEXT);

    ASSERT_EQ(toArrayIndex(COL_ID_WHITE_TEXT) + 1, console::TEXT_COLOR_CODES.size());
}
TEST(headerConsole, textBackground) {
    using namespace cth::console;

    setBGCol(COL_ID_BLACK_BG);
    print("i am black");
    setBGCol(COL_ID_DARK_RED_BG);
    print("i am dark red");
    setBGCol(COL_ID_DARK_GREEN_BG);
    print("i am dark green");
    setBGCol(COL_ID_DARK_YELLOW_BG);
    print("i am dark yellow");
    setBGCol(COL_ID_DARK_BLUE_BG);
    print("i am dark blue");
    setBGCol(COL_ID_DARK_MAGENTA_BG);
    print("i am dark magenta");
    setBGCol(COL_ID_DARK_CYAN_BG);
    print("i am dark cyan");
    setBGCol(COL_ID_DARK_WHITE_BG);
    print("i am dark white");
    setBGCol(COL_ID_BRIGHT_BLACK_BG);
    print("i am bright black");
    setBGCol(COL_ID_BRIGHT_RED_BG);
    print("i am bright red");
    setBGCol(COL_ID_BRIGHT_GREEN_BG);
    print("i am bright green");
    setBGCol(COL_ID_BRIGHT_YELLOW_BG);
    print("i am bright yellow");
    setBGCol(COL_ID_BRIGHT_BLUE_BG);
    print("i am bright blue");
    setBGCol(COL_ID_BRIGHT_MAGENTA_BG);
    print("i am bright magenta");
    setBGCol(COL_ID_BRIGHT_CYAN_BG);
    print("i am bright cyan");
    setBGCol(COL_ID_WHITE_BG);
    print("i am white");

    setBGCol(COL_ID_DEFAULT_BG);
    print("i am default");

    ASSERT_EQ(toArrayIndex(COL_ID_WHITE_BG) + 1, console::BG_COLOR_CODES.size());
}

TEST(headerConsole, textStyle) {
    using namespace cth::console;

    setTextStyle(STYLE_ID_BOLD_TEXT);
    println("i am BOLD_TEXT");
    setTextStyle(STYLE_ID_FAINT_TEXT);
    println("i am FAINT_TEXT");

    setTextStyle(STYLE_ID_NO_BOLD_TEXT);
    println("i am NO_BOLD_TEXT");

    setTextStyle(STYLE_ID_ITALIC_TEXT);
    println("i am ITALIC_TEXT");
    setTextStyle(STYLE_ID_NO_ITALIC_TEXT);
    println("i am NO_ITALIC_TEXT");

    setTextStyle(STYLE_ID_UNDERLINED_TEXT);
    println("i am UNDERLINED_TEXT");
    setTextStyle(STYLE_ID_DOUBLE_UNDERLINED_TEXT);
    println("i am DOUBLE_UNDERLINED_TEXT");
    setTextStyle(STYLE_ID_NO_UNDERLINED_TEXT);
    println("i am NO_UNDERLINED_TEXT");

    setTextStyle(STYLE_ID_BLINK_TEXT);
    println("i am BLINK_TEXT");
    setTextStyle(STYLE_ID_NO_BLINK_TEXT);
    println("i am NO_BLINK_TEXT");

    setTextStyle(STYLE_ID_INVERSE_TEXT);
    println("i am INVERSE_TEXT");
    setTextStyle(STYLE_ID_NO_INVERSE_TEXT);
    println("i am NO_INVERSE_TEXT");

    setTextStyle(STYLE_ID_HIDDEN_TEXT);
    println("i am HIDDEN_TEXT");
    setTextStyle(STYLE_ID_NO_HIDDEN_TEXT);
    println("i am NO_HIDDEN_TEXT");

    setTextStyle(STYLE_ID_STRIKEOUT_TEXT);
    println("i am STRIKEOUT_TEXT");
    setTextStyle(STYLE_ID_NO_STRIKEOUT_TEXT);
    println("i am NO_STRIKEOUT_TEXT");

    setTextStyle(STYLE_ID_BOLD_TEXT);
    setTextStyle(STYLE_ID_ITALIC_TEXT);
    setTextStyle(STYLE_ID_UNDERLINED_TEXT);
    setTextStyle(STYLE_ID_BLINK_TEXT);
    setTextStyle(STYLE_ID_INVERSE_TEXT);
    setTextStyle(STYLE_ID_HIDDEN_TEXT);
    setTextStyle(STYLE_ID_STRIKEOUT_TEXT);

    setTextStyle(STYLE_ID_DEFAULT_TEXT);
    println("i am DEFAULT TEXT");

    ASSERT_EQ(toArrayIndex(STYLE_ID_NO_STRIKEOUT_TEXT) + 1, console::TEXT_STYLE_CODES.size());
}

TEST(headerConsole, consoleState) {
    console::state::push();

    console::setTextCol(console::COL_ID_BRIGHT_RED_TEXT);
    console::setBGCol(console::COL_ID_BRIGHT_GREEN_BG);
    console::setTextStyle(console::STYLE_ID_BLINK_TEXT);

    console::print("i am bright red blink text with bright green BG");

    console::state::pop();
    console::println("");

    console::println("i am default text with default BG");
}

}
