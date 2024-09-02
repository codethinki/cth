// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once
#include "../cth_constants.hpp"
#include "../io/cth_log.hpp"

#include <memory>
#include <type_traits>



namespace cth {
template<class T, auto Null = nullptr> requires(
    std::equality_comparable_with<T, decltype(Null)> and
    not std::same_as<T, decltype(Null)>
)
class not_null {
    static constexpr bool NOEXCEPT_MOVE = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_move_constructible_v<T> : false;
    static constexpr bool NOEXCEPT_MOVE_ASSIGN = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_move_assignable_v<T> : false;
    static constexpr bool NOEXCEPT_ASSIGN = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_copy_assignable_v<T> : false;
    static constexpr bool NOEXCEPT_COPY = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_copy_constructible_v<T> : false;
    static constexpr bool COPY = std::is_copy_constructible_v<T>;

public:
    using null_t = decltype(Null);
    using type = T;


    CTH_RELEASE_CONSTEXPR not_null(T obj) noexcept(NOEXCEPT_MOVE) : _obj{std::move(obj)} {
#ifdef CTH_RELEASE_MODE
        if(obj == Null) std::unreachable();
#else
        CTH_ERR(obj == Null, "obj must not be Null") throw details->exception();
#endif
    }
    template<class V> requires std::convertible_to<V, T> and not std::same_as<V, T>
    constexpr not_null(not_null<V, Null> const& other) noexcept(NOEXCEPT_COPY) : _obj{other.get()} {}
    constexpr ~not_null() = default;

    constexpr [[nodiscard]] T release() noexcept(NOEXCEPT_ASSIGN) {
        T ptr{std::move(_obj)};
        _obj = Null;
        return ptr;
    }
    constexpr void swap(not_null& other) noexcept { std::swap(_obj, other._obj); }


    constexpr not_null(not_null const& other) = default;
    constexpr not_null& operator=(not_null const& other) = default;
    constexpr not_null(not_null&& other) noexcept(NOEXCEPT_MOVE) = default;
    constexpr not_null& operator=(not_null&& other) noexcept(NOEXCEPT_MOVE_ASSIGN) = default;


    constexpr not_null(null_t) = delete;
    not_null& operator=(null_t) = delete;
    [[nodiscard]] constexpr bool operator==(null_t) const = delete;
    [[nodiscard]] constexpr bool operator!=(null_t) const = delete;
    [[nodiscard]] constexpr bool operator<(null_t) const = delete;
    [[nodiscard]] constexpr bool operator>(null_t) const = delete;
    [[nodiscard]] constexpr bool operator<=(null_t) const = delete;
    [[nodiscard]] constexpr bool operator>=(null_t) const = delete;

private:
    T _obj;

public:
    [[nodiscard]] constexpr std::conditional_t<COPY, T, T const&> get() const noexcept { return _obj; }
    [[nodiscard]] constexpr decltype(auto) operator->() const { return get(); }
    [[nodiscard]] constexpr decltype(auto) operator*() const { return *get(); }

};

template<class T>
constexpr void swap(not_null<T>& left, not_null<T>& right) noexcept { left.swap(right); }

template<class T>
[[nodiscard]] auto make_not_null(T&& t) noexcept { return not_null<std::remove_cv_t<std::remove_reference_t<T>>>{std::forward<T>(t)}; }

template<class T, class U, auto Null>
[[nodiscard]] constexpr bool operator==(not_null<T, Null> const& ptr, not_null<U, Null> const& other) { return ptr.get() == other.get(); }
template<class T, class U, auto Null>
[[nodiscard]] constexpr bool operator!=(not_null<T, Null> const& left, not_null<U, Null> const& right) { return !(left.get() == right.get()); }
template<class T, class U, auto Null>
[[nodiscard]] constexpr bool operator<(not_null<T, Null> const& left, not_null<U, Null> const& right) { return std::less{}(left.get(), right.get()); }
template<class T, class U, auto Null>
[[nodiscard]] constexpr bool operator>(not_null<T, Null> const& left, not_null<U, Null> const& right) { return std::greater{}(left.get(), right.get()); }
template<class T, class U, auto Null>
[[nodiscard]] constexpr bool operator<=(not_null<T, Null> const& left, not_null<U, Null> const& right) { return std::less_equal{}(left.get(), right.get()); }
template<class T, class U, auto Null>
[[nodiscard]] constexpr bool operator>=(not_null<T, Null> const& left, not_null<U, Null> const& right) { return std::greater_equal{}(left.get(), right.get()); }

template<class T, auto Null>
[[nodiscard]] constexpr bool operator==(not_null<T, Null> const&, decltype(Null)) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator==(decltype(Null), not_null<T, Null> const&) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator!=(not_null<T, Null> const&, decltype(Null)) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator!=(decltype(Null), not_null<T, Null> const&) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator<(not_null<T, Null> const& left, decltype(Null)) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator<(decltype(Null), not_null<T, Null> const& right) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator>(not_null<T, Null> const& left, decltype(Null)) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator>(decltype(Null), not_null<T, Null> const& right) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator<=(not_null<T, Null> const& left, decltype(Null)) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator<=(decltype(Null), not_null<T, Null> const& right) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator>=(not_null<T, Null> const& left, decltype(Null)) = delete;
template<class T, auto Null>
[[nodiscard]] constexpr bool operator>=(decltype(Null), not_null<T, Null> const& right) = delete;

template<class T, auto Null>
std::ostream& operator<<(std::ostream& os, not_null<T, Null> const& not_null) {
    os << not_null.get();
    return os;
}

}

template<class T, auto Null>
struct std::hash<cth::not_null<T, Null>> /* NOLINT(cert-dcl58-cpp) this is a valid overload for std::hash */ {
    std::size_t operator()(cth::not_null<T, Null> const& ptr) const noexcept {
        using ptr_t = class cth::not_null<T, Null>::pointer;
        return std::hash<ptr_t>{}(ptr.get());
    }
};


namespace cth {
template<class T>
using unique_not_null = not_null<std::unique_ptr<T>>;
}
