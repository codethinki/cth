#include "test.hpp"
#define WIN_SCREEN_TEST(suite, test_name) WIN_EX_TEST(_screen, suite, test_name)

#include "cth/win/screen.hpp"

#include "cth/win/string.hpp"


namespace cth::win {

WIN_SCREEN_TEST(create_window, unicode) {
    std::string const name{"( ͡° ͜ʖ ͡°)🦣"};

    auto [classOpt, handle] = create_window(name, {}, false);

    EXPECT_TRUE(handle != nullptr);
    EXPECT_TRUE(classOpt.has_value() && classOpt->id == to_wstring(name));
}

WIN_SCREEN_TEST(create_window, ascii) {
    std::string const name{"asdf"};

    auto [classOpt, handle] = create_window(name, {}, false);

    EXPECT_TRUE(handle != nullptr);
    EXPECT_TRUE(classOpt.has_value() && classOpt->id == to_wstring(name));
}



WIN_SCREEN_TEST(monitors, main) { auto const monitors = enum_monitors(); }


}


namespace cth::win::screen {
WIN_SCREEN_TEST(desktop_rect, main) {
    auto const rect = desktop_rect();
    EXPECT_TRUE(rect.width > 0 && rect.height > 0);
}
WIN_SCREEN_TEST(window_rect, main) {
    auto const desktop = desktop_handle();

    auto const rect = window_rect(desktop);
    EXPECT_GT(rect.width, 0);
    EXPECT_GT(rect.height, 0);
}

}


/**
 * Converts a raw BGRA byte buffer into a visual ASCII/Unicode string.
 * Useful for debugging capture logic in the console.
 */
inline std::string to_ascii_view(std::span<std::byte> pixels, size_t width, size_t height) {
    // 1. Define the Palette (Dark -> Light)
    // You can change these characters to whatever Unicode you prefer.
    // Example: " ░▒▓█" for blocky look, or " .:-=+*#%@" for classic ASCII.
    static constexpr std::string_view PALETTE = " .:-=+*#%@";

    // 2. Pre-calculate size
    // We need 1 char per pixel + 1 newline per row
    std::string result;
    result.resize(height * (width + 1));

    size_t outIdx = 0;
    size_t const maxPaletteIdx = PALETTE.size() - 1;

    for(size_t y = 0; y < height; ++y) {
        for(size_t x = 0; x < width; ++x) {
            // 3. Calculate Input Index (BGRA = 4 bytes)
            size_t const idx = (y * width + x) * 4;

            // Safety check
            if(idx + 2 >= pixels.size()) break;

            // 4. Read Colors (Windows is B-G-R-A)
            int const b = std::to_integer<int>(pixels[idx]);
            int const g = std::to_integer<int>(pixels[idx + 1]);
            int const r = std::to_integer<int>(pixels[idx + 2]);

            // 5. Convert to Grayscale (Luminance)
            // Formula: 0.299R + 0.587G + 0.114B
            // Integer approximation: (R*30 + G*59 + B*11) / 100
            int const gray = (r * 30 + g * 59 + b * 11) / 100;

            // 6. Map Grayscale (0-255) to Palette Index
            int const charIdx = (gray * maxPaletteIdx) / 255;

            // 7. Write to string
            result[outIdx++] = PALETTE[charIdx];
        }
        // End of row
        result[outIdx++] = '\n';
    }

    return result;
}


namespace cth::win::screen {
WIN_SCREEN_TEST(section, read) {
    section section({100, 100, 10, 10});

    auto const view = section.capture();

    //simple check if not everything is black
    auto allBlack = std::ranges::all_of(view, [](auto const& e) { return e == std::byte{}; });

    EXPECT_FALSE(allBlack);
}
}
