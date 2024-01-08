#include "cth_numeric.hpp"
#include "cth_type_traits.hpp"
#include "cth_concepts.hpp"
#include "cth_console.hpp"


int main() {
    println("Hello, World!", cth::console::ANSI_ID_BRIGHT_BLACK_BG);
    println("Hello2", cth::console::ANSI_ID_CLEAR_UNDERLINE);


    return 0;
}
