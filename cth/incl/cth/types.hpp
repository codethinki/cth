#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cth {
/**
 * short names of the fixed width types
 * @details `using namespace cth::typ` imports only these names
 */
inline namespace typ {
    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using i8 = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using uz = std::size_t; ///< type of the `uz` literal
    using iz = std::make_signed_t<std::size_t>; ///< type of the `z` literal

    using f32 = float;
    using f64 = double;
}
}
