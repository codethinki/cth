#pragma once

#include <optional>
#include <type_traits>


//IMPLEMENT fix result storage to a more complete type and move to general cth/coro

namespace cth::co::dev {
template<class T>
struct result_storage {
    std::optional<T> value;

    constexpr result_storage() = default;

    template<class... Args>
    constexpr void emplace(Args&&... args) { value.emplace(std::forward<Args>(args)...); }

    constexpr void swap(result_storage& other) noexcept { value.swap(other.value); }

    template<class Self>
    constexpr auto&& operator*(this Self&& self) { return *std::forward<Self>(self).value; }

    [[nodiscard]] constexpr bool has_value() const noexcept { return value.has_value(); }
    [[nodiscard]] constexpr operator bool() const noexcept { return has_value(); }
};

template<>
struct result_storage<void> {
    constexpr void emplace() noexcept {}
    constexpr void swap(result_storage&) noexcept {}
    constexpr void operator*() const noexcept {}
};
}
