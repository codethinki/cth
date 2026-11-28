#include "cth/test.hpp"

#include "cth/io/keybd/keys.hpp"
#include "cth/types.hpp"

#include <format>


namespace cth::io {
namespace {
    /**
     * `true` if `to_utf8_string()` has a symbol for every `Key`
     * @details a missing case hits `std::unreachable()`, no constant expression
     */
    consteval bool every_key_has_a_symbol() {
        for(uz i = 0; i < keys_size(); ++i)
            if(to_utf8_string(static_cast<Key>(i)).empty())
                return false;
        return true;
    }

    consteval bool defaults_to_no_character_and_no_repeat() {
        key_state const fromExKey{ex_key{Key::A, false}, true};
        key_state const fromKey{Key::A, false, true};

        return fromExKey.character.empty() && !fromExKey.repeat
            && fromKey.character.empty() && !fromKey.repeat;
    }

    consteval bool stores_character_and_repeat() {
        key_state const fromExKey{ex_key{Key::A, false}, true, "a", true};
        key_state const fromKey{Key::A, false, true, "a", true};

        return fromExKey.character == "a" && fromExKey.repeat && fromKey == fromExKey;
    }

    consteval bool compares_the_character() {
        key_state const withoutCharacter{Key::A, false, true};
        key_state const withCharacter{Key::A, false, true, "a"};
        key_state const otherCharacter{Key::A, false, true, "A"};

        return withCharacter == key_state{Key::A, false, true, "a"}
            && withoutCharacter != withCharacter
            && withCharacter != otherCharacter;
    }

    consteval bool compares_repeat() {
        key_state const first{Key::A, false, true, "a"};
        key_state const repeat{Key::A, false, true, "a", true};

        return repeat == key_state{Key::A, false, true, "a", true} && first != repeat;
    }
}

CTH_TEST(Key, to_utf8_string_has_a_symbol_for_every_key) { static_assert(every_key_has_a_symbol()); }

CTH_TEST(Key, to_utf8_string_has_the_symbols_of_the_new_keys) {
    static_assert(to_utf8_string(Key::F13) == "F13");
    static_assert(to_utf8_string(Key::F24) == "F24");
    static_assert(to_utf8_string(Key::PRINT_SCREEN) == "PrtSc");
    static_assert(to_utf8_string(Key::SCROLL_LOCK) == "ScrLk");
    static_assert(to_utf8_string(Key::PAUSE) == "Pause");
    static_assert(to_utf8_string(Key::MENU) == "Menu");
    static_assert(to_utf8_string(Key::MOUSE_LEFT) == "LMB");
    static_assert(to_utf8_string(Key::MOUSE_RIGHT) == "RMB");
    static_assert(to_utf8_string(Key::MOUSE_MIDDLE) == "MMB");
    static_assert(to_utf8_string(Key::MOUSE_4) == "M4");
    static_assert(to_utf8_string(Key::MOUSE_5) == "M5");
}

CTH_TEST(key_state, defaults_to_no_character_and_no_repeat) {
    static_assert(defaults_to_no_character_and_no_repeat());
}

CTH_TEST(key_state, constructors_store_character_and_repeat) { static_assert(stores_character_and_repeat()); }

CTH_TEST(key_state, equality_compares_the_character) { static_assert(compares_the_character()); }

CTH_TEST(key_state, equality_compares_repeat) { static_assert(compares_repeat()); }

CTH_TEST(key_state, format_prints_character_and_repeat) {
    key_state const withCharacter{Key::A, false, true, "a", true};
    key_state const withoutCharacter{Key::SHIFT, true, false};

    EXPECT_EQ(
        std::format("{}", withCharacter),
        "cth::io::key_state{cth::io::ex_key{cth::io::Key{A}, right: false}, down: true, "
        R"(character: "a", repeat: true})"
    );
    EXPECT_EQ(
        std::format("{}", withoutCharacter),
        "cth::io::key_state{cth::io::ex_key{cth::io::Key{⇧}, right: true}, down: false, "
        R"(character: "", repeat: false})"
    );
}
}
