// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once
#include "../cth_constants.hpp"
#include "../io/cth_log.hpp"

#include <memory>
#include <type_traits>



namespace cth {
template<class T> requires(
    std::equality_comparable_with<T, nullptr_t> and
    not std::same_as<T, std::nullptr_t>
)
class not_null {
    static constexpr bool NOEXCEPT_MOVE = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_move_constructible_v<T> : false;
    static constexpr bool NOEXCEPT_MOVE_ASSIGN = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_move_assignable_v<T> : false;
    static constexpr bool NOEXCEPT_ASSIGN = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_copy_assignable_v<T> : false;
    static constexpr bool NOEXCEPT_COPY = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_copy_constructible_v<T> : false;
    static constexpr bool COPY = std::is_copy_constructible_v<T>;
public:
    using pointer = T;


    CTH_RELEASE_CONSTEXPR not_null(T ptr) noexcept(NOEXCEPT_MOVE) : _ptr{std::move(ptr)} {
#ifdef CTH_RELEASE_MODE
        if(ptr == nullptr) std::unreachable();
#else
        CTH_ERR(ptr == nullptr, "ptr must not be nullptr") throw details->exception();
#endif
    }
    template<class U> requires std::convertible_to<U, T> and not std::same_as<U, T>
    constexpr not_null(not_null<U> const& other) noexcept(NOEXCEPT_COPY) : _ptr{other.get()} {}
    constexpr ~not_null() = default;

    constexpr [[nodiscard]] T release() noexcept(NOEXCEPT_ASSIGN) {
        T ptr{std::move(_ptr)};
        _ptr = nullptr;
        return ptr;
    }
    constexpr void swap(not_null& other) noexcept { std::swap(_ptr, other._ptr); }


    constexpr not_null(not_null const& other) = default;
    constexpr not_null& operator=(not_null const& other) = default;
    constexpr not_null(not_null&& other) noexcept(NOEXCEPT_MOVE) = default;
    constexpr not_null& operator=(not_null&& other) noexcept(NOEXCEPT_MOVE_ASSIGN) = default;


    constexpr not_null(std::nullptr_t) = delete;
    not_null& operator=(std::nullptr_t) = delete;
    [[nodiscard]] constexpr bool operator==(std::nullptr_t) const = delete;
    [[nodiscard]] constexpr bool operator!=(std::nullptr_t) const = delete;
    [[nodiscard]] constexpr bool operator<(std::nullptr_t) const = delete;
    [[nodiscard]] constexpr bool operator>(std::nullptr_t) const = delete;
    [[nodiscard]] constexpr bool operator<=(std::nullptr_t) const = delete;
    [[nodiscard]] constexpr bool operator>=(std::nullptr_t) const = delete;

private:
    T _ptr;

public:
    [[nodiscard]] constexpr std::conditional_t<COPY, T, const T&> get() const noexcept { return _ptr; }
    [[nodiscard]] constexpr decltype(auto) operator->() const { return get(); }
    [[nodiscard]] constexpr decltype(auto) operator*() const { return *get(); }

};

template<typename T>
constexpr void swap(not_null<T>& left, not_null<T>& right) noexcept { left.swap(right); }

template<class T>
[[nodiscard]] auto make_not_null(T&& t) noexcept { return not_null<std::remove_cv_t<std::remove_reference_t<T>>>{std::forward<T>(t)}; }

template<typename T, typename U>
[[nodiscard]] constexpr bool operator==(not_null<T> const& ptr, not_null<U> const& other) { return ptr.get() == other.get(); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator!=(not_null<T> const& left, not_null<U> const& right) { return !(left.get() == right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<(not_null<T> const& left, not_null<U> const& right) { return std::less{}(left.get(), right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>(not_null<T> const& left, not_null<U> const& right) { return std::greater{}(left, right); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<=(not_null<T> const& left, not_null<U> const& right) { return std::less_equal{}(left, right); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>=(not_null<T> const& left, not_null<U> const& right) { return std::greater_equal{}(left, right); }

template<typename T>
[[nodiscard]] constexpr bool operator==(not_null<T> const&, std::nullptr_t) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t, not_null<T> const&) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator!=(not_null<T> const&, std::nullptr_t) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator!=(std::nullptr_t, not_null<T> const&) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator<(not_null<T> const& left, nullptr_t) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator<(nullptr_t, not_null<T> const& right) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator>(not_null<T> const& left, nullptr_t) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator>(nullptr_t, not_null<T> const& right) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator<=(not_null<T> const& left, nullptr_t) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator<=(nullptr_t, not_null<T> const& right) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator>=(not_null<T> const& left, nullptr_t) = delete;
template<typename T>
[[nodiscard]] constexpr bool operator>=(nullptr_t, not_null<T> const& right) = delete;

template<typename T>
std::ostream& operator<<(std::ostream& os, not_null<T> const& ptr) {
    os << ptr;
    return os;
}

}

template<typename T>
struct std::hash<cth::not_null<T>> /* NOLINT(cert-dcl58-cpp) this is a valid overload for std::hash */ {
    std::size_t operator()(cth::not_null<T> const& ptr) const noexcept {
        using ptr_t = typename cth::not_null<T>::pointer;
        return std::hash<ptr_t>{}(ptr.get());
    }
};


namespace cth {
template<class T>
using unique_not_null = not_null<std::unique_ptr<T>>;
}
