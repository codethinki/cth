#include "cth_numeric.hpp"
#include "cth_type_traits.hpp"
#include "cth_concepts.hpp"
#include "cth_console.hpp"


int main() {
    cth::console::println("Hello World!");
    cth::console::println("Hello, World!", cth::console::COL_ID_BRIGHT_RED_TEXT);
    cth::console::println("Hello, World!");

    cth::console::println("Hello, World!", cth::console::COL_ID_BRIGHT_GREEN_TEXT);
    cth::console::setBgCol(cth::console::COL_ID_BRIGHT_RED_BG);
    cth::console::errln(L"Hello, World!");

    return 0;
}
