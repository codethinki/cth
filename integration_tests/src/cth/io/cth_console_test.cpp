#include "test.hpp"

#include "cth/io/console.hpp"


namespace cth::io {

IO_TEST(console, asdf) {
    ansi_state state{};
    state.update(BGColor::BRIGHT_BLUE);
    auto defaultEncoded = ansi_state{}.encode();

    auto str = to_ansi_string(defaultEncoded);
    auto str2 = to_ansi_string(state.encode());

    std::println("{} asdf {}", str2, str);
}

IO_TEST(console, text_col) {
    using namespace cth::io;
    console.println(TextColor::BLACK, "i am black");
    console.println(TextColor::DARK_RED, "i am dark red");
    console.println(TextColor::DARK_GREEN, "i am dark green");
    console.println(TextColor::DARK_YELLOW, "i am dark yellow");
    console.println(TextColor::DARK_BLUE, "i am dark blue");
    console.println(TextColor::DARK_MAGENTA, "i am dark magenta");
    console.println(TextColor::DARK_CYAN, "i am dark cyan");
    console.println(TextColor::DARK_WHITE, "i am dark white");
    console.println(TextColor::BRIGHT_BLACK, "i am bright black");
    console.println(TextColor::BRIGHT_RED, "i am bright red");
    console.println(TextColor::BRIGHT_GREEN, "i am bright green");
    console.println(TextColor::BRIGHT_YELLOW, "i am bright yellow");
    console.println(TextColor::BRIGHT_BLUE, "i am bright blue");
    console.println(TextColor::BRIGHT_MAGENTA, "i am bright magenta");
    console.println(TextColor::BRIGHT_CYAN, "i am bright cyan");
    console.println(TextColor::WHITE, "i am white");

    console.println(TextColor::DEFAULT, "i am default");
}
IO_TEST(console, background) {
    using namespace cth::io;

    error.update(BGColor::BLACK);
    error.print("i am black bg");
    error.update(BGColor::DARK_RED);
    error.print("i am dark red bg");
    error.update(BGColor::DARK_GREEN);
    error.print("i am dark green bg");
    error.update(BGColor::DARK_YELLOW);
    error.print("i am dark yellow bg");
    error.update(BGColor::DARK_BLUE);
    error.print("i am dark blue bg");
    error.update(BGColor::DARK_MAGENTA);
    error.print("i am dark magenta bg");
    error.update(BGColor::DARK_CYAN);
    error.print("i am dark cyan bg");
    error.update(BGColor::DARK_WHITE);
    error.print("i am dark white bg");
    error.update(BGColor::BRIGHT_BLACK);
    error.print("i am bright black bg");
    error.update(BGColor::BRIGHT_RED);
    error.print("i am bright red bg");
    error.update(BGColor::BRIGHT_GREEN);
    error.print("i am bright green bg");
    error.update(BGColor::BRIGHT_YELLOW);
    error.print("i am bright yellow bg");
    error.update(BGColor::BRIGHT_BLUE);
    error.print("i am bright blue bg");
    error.update(BGColor::BRIGHT_MAGENTA);
    error.print("i am bright magenta bg");
    error.update(BGColor::BRIGHT_CYAN);
    error.print("i am bright cyan bg");
    error.update(BGColor::WHITE);
    error.print("i am white bg");

    error.update(BGColor::DEFAULT);
    error.print("i am default bg");
    console.println("");
}

IO_TEST(console, text_style) {
    using namespace cth::io;
    error.update(TextIntensity::BOLD);
    error.println("i am BOLD_TEXT");
    error.update(TextIntensity::NORMAL);
    error.println("i am no BOLD_TEXT");

    error.update(TextIntensity::FAINT);
    error.println("i am FAINT_TEXT");
    error.update(TextIntensity::NORMAL);
    error.println("i am no FAINT_TEXT");

    error.update(TextModifiers::ITALIC, true);
    error.println("i am ITALIC_TEXT");
    error.update(TextModifiers::ITALIC, false);
    error.println("i am no ITALIC_TEXT");

    error.update(TextUnderline::SINGLE);
    error.println("i am UNDERLINED_TEXT");
    error.update(TextUnderline::NONE);
    error.println("i am no UNDERLINED_TEXT");

    error.update(TextUnderline::DOUBLE);
    error.update(TextUnderline::SINGLE);
    error.println("i am normal text");

    error.update(TextUnderline::DOUBLE);
    error.println("i am DOUBLE_UNDERLINED_TEXT");
    error.update(TextUnderline::NONE);
    error.println("i am no UNDERLINED_TEXT");

    error.update(TextModifiers::BLINK, true);
    error.println("i am BLINK_TEXT");
    error.update(TextModifiers::BLINK, false);
    error.println("i am no BLINK_TEXT");

    error.update(TextModifiers::INVERSE, true);
    error.println("i am INVERSE_TEXT");
    error.update(TextModifiers::INVERSE, false);
    error.println("i am no INVERSE_TEXT");

    error.update(TextModifiers::HIDDEN, true);
    error.print("i am HIDDEN_TEXT");
    error.update(TextModifiers::HIDDEN, false);
    console.println(" (<-- should be hidden)");
    std::println("i am no HIDDEN_TEXT");

    error.update(TextModifiers::STRIKEOUT, true);
    error.println("i am STRIKEOUT_TEXT");
    error.update(TextModifiers::STRIKEOUT, false);
    error.println("i am no STRIKEOUT_TEXT");

    error.update(TextIntensity::BOLD);
    error.update(TextModifiers::ITALIC, true);
    error.update(TextUnderline::SINGLE);
    error.update(TextModifiers::BLINK, true);
    error.update(TextModifiers::INVERSE, true);
    error.update(TextModifiers::HIDDEN, true);
    error.update(TextModifiers::STRIKEOUT, true);

    error.update(ansi_state{});
    error.println("i am DEFAULT_TEXT");
}

IO_TEST(console, state) {
    using namespace cth::io;
    error.pushState();

    error.update(TextColor::BRIGHT_RED);
    error.update(BGColor::BRIGHT_GREEN);
    error.update(TextModifiers::BLINK, true);

    error.print("i am bright red blink text with bright green BG");

    error.popState();
    std::println();

    std::println("i am default text with default BG");
}

}
