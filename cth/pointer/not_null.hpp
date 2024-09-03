// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once
#include "../cth_constants.hpp"
#include "../io/cth_log.hpp"

#include <memory>
#include <type_traits>
#include <vector>



namespace cth {
template<class T, auto Null = nullptr, bool Adapt = false> requires(
    std::equality_comparable_with<T, decltype(Null)>
)
class not_null {
    static constexpr bool NOEXCEPT_MOVE = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_move_constructible_v<T> : false;
    static constexpr bool NOEXCEPT_MOVE_ASSIGN = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_move_assignable_v<T> : false;
    static constexpr bool NOEXCEPT_ASSIGN = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_copy_assignable_v<T> : false;
    static constexpr bool NOEXCEPT_COPY = COMPILATION_MODE == MODE_RELEASE ? std::is_nothrow_copy_constructible_v<T> : false;
    static constexpr bool COPY = std::is_trivially_copyable_v<T> && sizeof(T) <= 16;
    static constexpr bool ADAPT_RELEASE = Adapt && cth::type::has_release<T>;
    static constexpr bool ADAPT_GET = Adapt && cth::type::has_get<T>;

    static CTH_RELEASE_CONSTEXPR void checkValue(T const& obj) noexcept(COMPILATION_MODE != MODE_DEBUG) {
        if constexpr(COMPILATION_MODE == MODE_DEBUG) { CTH_ERR(obj == Null, "obj must not be Null") throw details->exception(); } else
            std::unreachable();
    }

public:
    using null_t = decltype(Null);
    using type = T;


    CTH_RELEASE_CONSTEXPR not_null(T obj) noexcept(NOEXCEPT_MOVE) : _obj{std::move(obj)} { not_null::checkValue(_obj); }
    template<class U> requires (std::convertible_to<U, T> and not std::same_as<U, T>)
    constexpr not_null(not_null<U, Null, Adapt> const& other) noexcept(NOEXCEPT_COPY) : _obj{other.get()} {}
    constexpr ~not_null() = default;

    constexpr [[nodiscard]] T release_val() noexcept(NOEXCEPT_ASSIGN) {
        T obj{std::move(_obj)};
        _obj = Null;
        return obj;
    }
    [[nodiscard]] constexpr auto release() noexcept(NOEXCEPT_ASSIGN) {
        if constexpr (!ADAPT_RELEASE) return release_val();

        auto obj{std::move(_obj.get())};
        _obj = Null;
        return obj;
    }



    constexpr void swap(not_null& other) noexcept { std::swap(_obj, other._obj); }


    constexpr not_null(not_null const& other) = default;
    constexpr not_null& operator=(not_null const& other) requires (not std::same_as<T, null_t>) = default;

    constexpr not_null(not_null&& other) noexcept(NOEXCEPT_MOVE) = default;
    constexpr not_null& operator=(not_null&& other) noexcept(NOEXCEPT_MOVE_ASSIGN) requires(std::same_as<T, null_t>) = default;

    constexpr not_null(null_t) requires (not std::same_as<T, null_t>) = delete;

    not_null& operator=(null_t) requires (not std::same_as<T, null_t>) = delete;

    [[nodiscard]] constexpr bool operator==(null_t) const requires (not std::same_as<T, null_t>) = delete;
    [[nodiscard]] constexpr bool operator!=(null_t) const requires (not std::same_as<T, null_t>) = delete;
    [[nodiscard]] constexpr bool operator<(null_t) const requires (not std::same_as<T, null_t>) = delete;
    [[nodiscard]] constexpr bool operator>(null_t) const requires (not std::same_as<T, null_t>) = delete;
    [[nodiscard]] constexpr bool operator<=(null_t) const requires (not std::same_as<T, null_t>) = delete;
    [[nodiscard]] constexpr bool operator>=(null_t) const requires (not std::same_as<T, null_t>) = delete;

private:
    T _obj;

public:
    [[nodiscard]] constexpr auto get() const noexcept {
        if constexpr(ADAPT_GET) return _obj.get();
        else return _obj;
    }
    [[nodiscard]] constexpr std::conditional_t<COPY, T, T const&> get_val() const noexcept { return _obj; }
    [[nodiscard]] constexpr decltype(auto) operator->() const { return get(); }
    [[nodiscard]] constexpr decltype(auto) operator*() const requires(cth::type::has_deref<T>) { return *get(); }
};


template<class T>
constexpr void swap(not_null<T>& left, not_null<T>& right) noexcept { left.swap(right); }

template<class T, auto Null = nullptr, bool Adapt>
[[nodiscard]] auto make_not_null(T&& t) noexcept { return not_null<std::remove_cv_t<std::remove_reference_t<T>>, Null, Adapt>{std::forward<T>(t)}; }

template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] constexpr bool operator==(not_null<T, Null, Adapt> const& ptr, not_null<U, Null, Adapt> const& other) {
    return std::equal_to{}(ptr.get(), other.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] constexpr bool operator!=(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::not_equal_to{}(left.get(), right.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] constexpr bool operator<(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::less{}(left.get(), right.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] constexpr bool operator>(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::greater{}(left.get(), right.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] constexpr bool operator<=(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::less_equal{}(left.get(), right.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] constexpr bool operator>=(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::greater_equal{}(left.get(), right.get());
}



template<class T, auto Null, bool Adapt>
std::ostream& operator<<(std::ostream& os, not_null<T, Null, Adapt> const& not_null) {
    os << not_null.get();
    return os;
}

template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator==(not_null<T, Null, Adapt> const&, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator==(decltype(Null), not_null<T, Null, Adapt> const&) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator!=(not_null<T, Null, Adapt> const&, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator!=(decltype(Null), not_null<T, Null, Adapt> const&) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator<(not_null<T, Null, Adapt> const& left, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator<(decltype(Null), not_null<T, Null, Adapt> const& right) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator>(not_null<T, Null, Adapt> const& left, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator>(decltype(Null), not_null<T, Null, Adapt> const& right) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator<=(not_null<T, Null, Adapt> const& left, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator<=(decltype(Null), not_null<T, Null, Adapt> const& right) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator>=(not_null<T, Null, Adapt> const& left, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (not std::same_as<T, decltype(Null)>)
[[nodiscard]] constexpr bool operator>=(decltype(Null), not_null<T, Null, Adapt> const& right) = delete;


}

template<class T, auto Null, bool Adapt>
struct std::hash<cth::not_null<T, Null, Adapt>> /* NOLINT(cert-dcl58-cpp) this is a valid overload for std::hash */ {
    std::size_t operator()(cth::not_null<T, Null, Adapt> const& obj) const noexcept {
        using ptr_t = class cth::not_null<T, Null, Adapt>::pointer;
        return std::hash<ptr_t>{}(obj.get());
    }
};


namespace cth {
template<class T>
using unique_not_null = not_null<std::unique_ptr<T>, nullptr, true>;

template<class T>
not_null(std::unique_ptr<T>) -> unique_not_null<T>;

template<class T>
not_null(unique_not_null<T>) -> unique_not_null<T>;

}
