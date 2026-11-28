#include "cth/test.hpp"

#include "cth/types.hpp"

#include <type_traits>


namespace cth {
CTH_TEST(typ, aliases_the_fixed_width_types) {
    static_assert(std::is_same_v<u8, std::uint8_t> && std::is_same_v<u16, std::uint16_t>);
    static_assert(std::is_same_v<u32, std::uint32_t> && std::is_same_v<u64, std::uint64_t>);
    static_assert(std::is_same_v<i8, std::int8_t> && std::is_same_v<i16, std::int16_t>);
    static_assert(std::is_same_v<i32, std::int32_t> && std::is_same_v<i64, std::int64_t>);
    static_assert(std::is_same_v<f32, float> && std::is_same_v<f64, double>);
}

CTH_TEST(typ, size_types_are_the_literal_types) {
    static_assert(std::is_same_v<uz, decltype(0uz)>);
    static_assert(std::is_same_v<iz, decltype(0z)>);
    static_assert(std::is_same_v<uz, std::size_t>);
}

CTH_TEST(typ, address_types_are_the_std_address_types) {
    static_assert(std::is_same_v<uaddr, std::uintptr_t> && std::is_same_v<iaddr, std::intptr_t>);
}
}

namespace {
using namespace cth::typ;

CTH_EX_TEST(_typ, typ, using_directive_imports_the_types) {
    static_assert(std::is_same_v<u32, cth::u32>);
    static_assert(std::is_same_v<uz, cth::uz>);
}
}
