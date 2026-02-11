#include "test.hpp"

#include "cth/io/console.hpp"


namespace cth::io {
using std::print;
using std::println;

IO_TEST(console, text_col) {
    using namespace cth::io;
    console.println(BLACK_TEXT_COL, "i am black");
    console.println(DARK_RED_TEXT_COL, "i am dark red");
    console.println(DARK_GREEN_TEXT_COL, "i am dark green");
    console.println(DARK_YELLOW_TEXT_COL, "i am dark yellow");
    console.println(DARK_BLUE_TEXT_COL, "i am dark blue");
    console.println(DARK_MAGENTA_TEXT_COL, "i am dark magenta");
    console.println(DARK_CYAN_TEXT_COL, "i am dark cyan");
    console.println(DARK_WHITE_TEXT_COL, "i am dark white");
    console.println(BRIGHT_BLACK_TEXT_COL, "i am bright black");
    console.println(BRIGHT_RED_TEXT_COL, "i am bright red");
    console.println(BRIGHT_GREEN_TEXT_COL, "i am bright green");
    console.println(BRIGHT_YELLOW_TEXT_COL, "i am bright yellow");
    console.println(BRIGHT_BLUE_TEXT_COL, "i am bright blue");
    console.println(BRIGHT_MAGENTA_TEXT_COL, "i am bright magenta");
    console.println(BRIGHT_CYAN_TEXT_COL, "i am bright cyan");
    console.println(WHITE_TEXT_COL, "i am white");

    console.println(DEFAULT_TEXT_COL, "i am default");
}
IO_TEST(console, background) {
    using namespace cth::io;


    error.setBGCol(BLACK_BG_COL);
    print("i am black");
    error.setBGCol(DARK_RED_BG_COL);
    print("i am dark red");
    error.setBGCol(DARK_GREEN_BG_COL);
    print("i am dark green");
    error.setBGCol(DARK_YELLOW_BG_COL);
    print("i am dark yellow");
    error.setBGCol(DARK_BLUE_BG_COL);
    print("i am dark blue");
    error.setBGCol(DARK_MAGENTA_BG_COL);
    print("i am dark magenta");
    error.setBGCol(DARK_CYAN_BG_COL);
    print("i am dark cyan");
    error.setBGCol(DARK_WHITE_BG_COL);
    print("i am dark white");
    error.setBGCol(BRIGHT_BLACK_BG_COL);
    print("i am bright black");
    error.setBGCol(BRIGHT_RED_BG_COL);
    print("i am bright red");
    error.setBGCol(BRIGHT_GREEN_BG_COL);
    print("i am bright green");
    error.setBGCol(BRIGHT_YELLOW_BG_COL);
    print("i am bright yellow");
    error.setBGCol(BRIGHT_BLUE_BG_COL);
    print("i am bright blue");
    error.setBGCol(BRIGHT_MAGENTA_BG_COL);
    print("i am bright magenta");
    error.setBGCol(BRIGHT_CYAN_BG_COL);
    print("i am bright cyan");
    error.setBGCol(WHITE_BG_COL);
    print("i am white");

    error.setBGCol(DEFAULT_BG_COL);
    print("i am default");
    console.println("");
}

IO_TEST(console, text_style) {
    using namespace cth::io;
    error.setTextStyle(BOLD_TEXT_STYLE);
    println("i am BOLD_TEXT");
    error.setTextStyle(BOLD_TEXT_STYLE, false);
    println("i am no BOLD_TEXT");

    error.setTextStyle(FAINT_TEXT_STYLE);
    println("i am FAINT_TEXT");
    error.setTextStyle(FAINT_TEXT_STYLE, false);
    println("i am no FAINT_TEXT");

    error.setTextStyle(ITALIC_TEXT_STYLE);
    println("i am ITALIC_TEXT");
    error.setTextStyle(ITALIC_TEXT_STYLE, false);
    println("i am no ITALIC_TEXT");

    error.setTextStyle(UNDERLINED_TEXT_STYLE);
    println("i am UNDERLINED_TEXT");
    error.setTextStyle(UNDERLINED_TEXT_STYLE, false);
    println("i am no UNDERLINED_TEXT");

    error.setTextStyle(DOUBLE_UNDERLINED_TEXT_STYLE);
    error.setTextStyle(UNDERLINED_TEXT_STYLE);
    println("i am normal text");

    error.setTextStyle(DOUBLE_UNDERLINED_TEXT_STYLE);
    println("i am DOUBLE_UNDERLINED_TEXT");
    error.setTextStyle(DOUBLE_UNDERLINED_TEXT_STYLE, false);
    println("i am no UNDERLINED_TEXT");

    error.setTextStyle(BLINK_TEXT_STYLE);
    println("i am BLINK_TEXT");
    error.setTextStyle(BLINK_TEXT_STYLE, false);
    println("i am no BLINK_TEXT");

    error.setTextStyle(INVERSE_TEXT_STYLE);
    println("i am INVERSE_TEXT");
    error.setTextStyle(INVERSE_TEXT_STYLE, false);
    println("i am no INVERSE_TEXT");

    error.setTextStyle(HIDDEN_TEXT_STYLE);
    print("i am HIDDEN_TEXT");
    error.setTextStyle(HIDDEN_TEXT_STYLE, false);
    console.println(" (<-- should be hidden)");
    println("i am no HIDDEN_TEXT");

    error.setTextStyle(STRIKEOUT_TEXT_STYLE);
    println("i am STRIKEOUT_TEXT");
    error.setTextStyle(STRIKEOUT_TEXT_STYLE);
    println("i am no STRIKEOUT_TEXT");

    error.setTextStyle(BOLD_TEXT_STYLE);
    error.setTextStyle(ITALIC_TEXT_STYLE);
    error.setTextStyle(UNDERLINED_TEXT_STYLE);
    error.setTextStyle(BLINK_TEXT_STYLE);
    error.setTextStyle(INVERSE_TEXT_STYLE);
    error.setTextStyle(HIDDEN_TEXT_STYLE);
    error.setTextStyle(STRIKEOUT_TEXT_STYLE);

    error.resetState();
    println("i am DEFAULT_TEXT");
}

IO_TEST(console, state) {
    using namespace cth::io;
    error.pushState();

    error.setTextCol(BRIGHT_RED_TEXT_COL);
    error.setBGCol(io::BRIGHT_GREEN_BG_COL);
    error.setTextStyle(io::BLINK_TEXT_STYLE);

    print("i am bright red blink text with bright green BG");

    error.popState();
    println("");

    println("i am default text with default BG");
}

}
