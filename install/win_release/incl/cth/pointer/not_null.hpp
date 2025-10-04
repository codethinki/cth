// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once
#include "../constants.hpp"
#include "../io/log.hpp"

#include <memory>
#include <type_traits>
#include <vector>



namespace cth {
template<class T, auto Null = nullptr, bool Adapt = false> requires(
    std::equality_comparable_with<T, decltype(Null)>
)
class not_null {
public:
    using null_t = decltype(Null);
    using type = T;
    static cxpr bool NULL_OP = std::same_as<T, null_t>;

private:
    static cxpr bool CPY_CTOR = std::is_copy_constructible_v<T>;
    static cxpr bool CPY_ASSIGN = std::is_copy_assignable_v<T>;
    static cxpr bool MOVE_CTOR = std::is_move_constructible_v<T>;
    static cxpr bool MOVE_ASSIGN = std::is_move_assignable_v<T>;

    static cxpr bool NULL_CPY_CTOR = NULL_OP && CPY_CTOR;
    static cxpr bool NULL_CPY_ASSIGN = NULL_OP && CPY_ASSIGN;
    static cxpr bool NULL_MOVE_CTOR = NULL_OP && MOVE_CTOR;
    static cxpr bool NULL_MOVE_ASSIGN = NULL_OP && MOVE_ASSIGN;

    static cxpr bool NOEXCEPT_MOVE_CTOR = std::is_nothrow_move_constructible_v<T>;
    static cxpr bool NOEXCEPT_MOVE_ASSIGN = std::is_nothrow_move_assignable_v<T>;
    static cxpr bool NOEXCEPT_COPY_CTOR = std::is_nothrow_copy_constructible_v<T>;
    static cxpr bool NOEXCEPT_ASSIGN = std::is_nothrow_copy_assignable_v<T>;

    static cxpr bool GET_COPY = std::is_trivially_copyable_v<T> && sizeof(T) <= 16;
    static cxpr bool ADAPT_RELEASE = Adapt && cth::type::has_release<T>;
    static cxpr bool ADAPT_GET = Adapt && cth::type::has_get<T>;

    static cxpr bool NOEXCEPT_CHECK_VAL = COMPILATION_MODE != CompilationMode::DEBUG;
    static cxpr bool RELEASE_VAL_NOEXCEPT = NOEXCEPT_ASSIGN && NOEXCEPT_MOVE_CTOR;

    static CTH_RELEASE_CONSTEXPR void checkVal(T const& obj) noexcept(NOEXCEPT_CHECK_VAL) {
        CTH_CRITICAL(obj == Null, "obj must not be Null") {}
    }

public:
    CTH_RELEASE_CONSTEXPR not_null(T obj) noexcept(NOEXCEPT_MOVE_CTOR && NOEXCEPT_CHECK_VAL) : _obj{std::move(obj)} { not_null::checkVal(_obj); }
    template<class U> requires (std::convertible_to<U, T> and not std::same_as<U, T>)
    cxpr not_null(not_null<U, Null, Adapt> const& other) noexcept(NOEXCEPT_COPY_CTOR) requires(CPY_CTOR) : _obj{other.get_val()} {}
    cxpr ~not_null() = default;

    [[nodiscard]] cxpr T release_val() noexcept(RELEASE_VAL_NOEXCEPT) {
        T obj{std::move(_obj)};
        _obj = Null;
        return obj;
    }
    [[nodiscard]] cxpr auto
    release() noexcept((!ADAPT_RELEASE && RELEASE_VAL_NOEXCEPT) || (ADAPT_RELEASE && noexcept(std::declval<T>().release()))) {
        if cxpr(!ADAPT_RELEASE) return release_val();

        return _obj.release();
    }



    cxpr void swap(not_null& other) noexcept { std::swap(_obj, other._obj); }


    cxpr not_null(not_null const& other) requires(CPY_CTOR) = default;
    cxpr not_null(not_null const& other) requires (!CPY_CTOR) = delete;
    cxpr not_null& operator=(not_null const& other) requires (CPY_ASSIGN) = default;
    cxpr not_null& operator=(not_null const& other) requires (!CPY_ASSIGN) = delete;

    cxpr not_null(not_null&& other) noexcept(NOEXCEPT_MOVE_CTOR) requires (MOVE_CTOR) = default;
    cxpr not_null(not_null&& other) noexcept(NOEXCEPT_MOVE_CTOR) requires (!MOVE_CTOR) = delete;
    cxpr not_null& operator=(not_null&& other) noexcept(NOEXCEPT_MOVE_ASSIGN) requires(MOVE_ASSIGN) = default;
    cxpr not_null& operator=(not_null&& other) noexcept(NOEXCEPT_MOVE_ASSIGN) requires(!MOVE_ASSIGN) = delete;



    cxpr not_null(null_t const&) requires (!NULL_CPY_CTOR) = delete;
    cxpr not_null& operator=(null_t const&) requires (NULL_CPY_ASSIGN) = delete;
    cxpr not_null(null_t&& other) requires (!NULL_MOVE_CTOR) = delete;
    cxpr not_null& operator=(null_t&& other) requires(!NULL_MOVE_ASSIGN) = delete;



    [[nodiscard]] cxpr bool operator==(null_t) const requires (!NULL_OP) = delete;
    [[nodiscard]] cxpr bool operator!=(null_t) const requires (!NULL_OP) = delete;
    [[nodiscard]] cxpr bool operator<(null_t) const requires (!NULL_OP) = delete;
    [[nodiscard]] cxpr bool operator>(null_t) const requires (!NULL_OP) = delete;
    [[nodiscard]] cxpr bool operator<=(null_t) const requires (!NULL_OP) = delete;
    [[nodiscard]] cxpr bool operator>=(null_t) const requires (!NULL_OP) = delete;

private:
    T _obj;

public:
    [[nodiscard]] cxpr auto get() const noexcept {
        if constexpr(ADAPT_GET) return _obj.get();
        else return get_val();
    }
    [[nodiscard]] cxpr std::conditional_t<GET_COPY, T, T const&> get_val() const noexcept { return _obj; }
    [[nodiscard]] cxpr dclauto operator->() const { return get(); }
    [[nodiscard]] cxpr dclauto operator*() const requires(cth::type::has_deref<T>) { return *get(); }
};


template<class T>
cxpr void swap(not_null<T>& left, not_null<T>& right) noexcept { left.swap(right); }

template<class T, auto Null = nullptr, bool Adapt>
[[nodiscard]] auto make_not_null(T&& t) noexcept { return not_null<std::remove_cv_t<std::remove_reference_t<T>>, Null, Adapt>{std::forward<T>(t)}; }

template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] cxpr bool operator==(not_null<T, Null, Adapt> const& ptr, not_null<U, Null, Adapt> const& other) {
    return std::equal_to{}(ptr.get(), other.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] cxpr bool operator!=(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::not_equal_to{}(left.get(), right.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] cxpr bool operator<(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::less{}(left.get(), right.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] cxpr bool operator>(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::greater{}(left.get(), right.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] cxpr bool operator<=(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::less_equal{}(left.get(), right.get());
}
template<class T, class U, auto Null, bool Adapt>
[[nodiscard]] cxpr bool operator>=(not_null<T, Null, Adapt> const& left, not_null<U, Null, Adapt> const& right) {
    return std::greater_equal{}(left.get(), right.get());
}



template<class T, auto Null, bool Adapt>
std::ostream& operator<<(std::ostream& os, not_null<T, Null, Adapt> const& not_null) {
    os << not_null.get();
    return os;
}

template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator==(not_null<T, Null, Adapt> const&, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator==(decltype(Null), not_null<T, Null, Adapt> const&) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator!=(not_null<T, Null, Adapt> const&, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator!=(decltype(Null), not_null<T, Null, Adapt> const&) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator<(not_null<T, Null, Adapt> const& left, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator<(decltype(Null), not_null<T, Null, Adapt> const& right) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator>(not_null<T, Null, Adapt> const& left, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator>(decltype(Null), not_null<T, Null, Adapt> const& right) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator<=(not_null<T, Null, Adapt> const& left, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator<=(decltype(Null), not_null<T, Null, Adapt> const& right) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator>=(not_null<T, Null, Adapt> const& left, decltype(Null)) = delete;
template<class T, auto Null, bool Adapt> requires (!not_null<T, Null, Adapt>::NULL_OP)
[[nodiscard]] cxpr bool operator>=(decltype(Null), not_null<T, Null, Adapt> const& right) = delete;


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
using unique_not_null = cth::not_null<std::unique_ptr<T>, nullptr, true>;

template<class T>
not_null(std::unique_ptr<T>) -> cth::unique_not_null<T>;

template<class T>
not_null(cth::unique_not_null<T>) -> cth::unique_not_null<T>;
}
