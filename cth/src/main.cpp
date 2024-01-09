#include "cth_numeric.hpp"
#include "cth_type_traits.hpp"
#include "cth_concepts.hpp"
#include "cth_console.hpp"


int main() {
    cth::Console::println("Hello World!");
    cth::Console::println("Hello, World!", cth::Console::COL_ID_BRIGHT_RED_TEXT);
    cth::Console::println("Hello, World!");

    cth::Console::println("Hello, World!", cth::Console::COL_ID_BRIGHT_GREEN_TEXT);
    cth::Console::setBgCol(cth::Console::COL_ID_BRIGHT_RED_BG);
    cth::Console::errln(L"Hello, World!");

    return 0;
}
