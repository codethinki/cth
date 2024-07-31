#pragma once
#include <memory>
#include <type_traits>



namespace cth {
/**
 * \brief move aware pointer (nullptr after move)
 * \note NOT memory safe
 */
template<typename T>
struct move_ptr {
    using pointer = T*;
    using element_type = T;
    constexpr move_ptr() noexcept = default;
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr move_ptr(T* ptr) : _ptr(ptr) {}
    constexpr ~move_ptr() = default;

private:
    T* _ptr = nullptr;

public:
    constexpr void free() noexcept {
        delete _ptr;
        _ptr = nullptr;
    }

    constexpr void swap(move_ptr& other) noexcept { std::swap(_ptr, other._ptr); }

    [[nodiscard]] constexpr T* get() const { return _ptr; }

    [[nodiscard]] constexpr T* release() noexcept {
        auto ptr = _ptr;
        _ptr = nullptr;
        return ptr;
    }
    [[nodiscard]] constexpr T* operator->() const { return _ptr; }
    [[nodiscard]] constexpr T& operator*() const { return *_ptr; }
    [[nodiscard]] explicit constexpr operator bool() const { return static_cast<bool>(_ptr); }

   /* template<typename U> requires std::convertible_to<U*, T*> and !std::same_as<U, T>
    explicit constexpr move_ptr(const move_ptr<U>& other) noexcept : _ptr(other.get()) {}*/

    move_ptr(move_ptr const& other) = default;
    move_ptr& operator=(move_ptr const& other) = default;

    move_ptr(move_ptr&& other) noexcept : _ptr(other.release()) {}
    move_ptr& operator=(move_ptr&& other) noexcept {
        _ptr = other.release();
        return *this;
    }


};
template<typename T>
constexpr void swap(move_ptr<T>& left, move_ptr<T>& right) noexcept { left.swap(right); }

template<typename T, typename... Args>
[[nodiscard]] constexpr move_ptr<T> make_basic(Args&&... args) { return move_ptr<T>(new T(std::forward<Args>(args)...)); }

template<typename T, typename U>
[[nodiscard]] constexpr bool operator==(move_ptr<T> const& ptr, move_ptr<U> const& other) { return ptr.get() == other.get(); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator!=(move_ptr<T> const& left, move_ptr<U> const& right) { return !(left.get() == right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<(move_ptr<T> const& left, move_ptr<U> const& right) {
    using left_t = typename move_ptr<T>::pointer;
    using right_t = typename move_ptr<U>::pointer;
    using ptr_t = std::common_type_t<left_t, right_t>;

    return std::less<ptr_t>{}(left.get(), right.get());
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>(move_ptr<T> const& left, move_ptr<U> const& right) { return right < left; }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<=(move_ptr<T> const& left, move_ptr<U> const& right) { return !(right < left); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>=(move_ptr<T> const& left, move_ptr<U> const& right) { return !(left < right); }

template<typename T>
[[nodiscard]] constexpr bool operator==(move_ptr<T> const& ptr, std::nullptr_t) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t, move_ptr<T> const& ptr) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(move_ptr<T> const& ptr, std::nullptr_t) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(std::nullptr_t, move_ptr<T> const& ptr) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator<(move_ptr<T> const& left, nullptr_t) {
    using ptr_t = typename move_ptr<T>::pointer;
    return std::less<ptr_t>{}(left.get(), nullptr);
}
template<typename T>
[[nodiscard]] constexpr bool operator<(nullptr_t, move_ptr<T> const& right) {
    using ptr_t = typename move_ptr<T>::pointer;
    return std::less<ptr_t>{}(nullptr, right.get());
}
template<typename T>
[[nodiscard]] constexpr bool operator>(move_ptr<T> const& left, nullptr_t) { return nullptr < left; }
template<typename T>
[[nodiscard]] constexpr bool operator>(nullptr_t, move_ptr<T> const& right) { return right < nullptr; }
template<typename T>
[[nodiscard]] constexpr bool operator<=(move_ptr<T> const& left, nullptr_t) { return !(nullptr < left); }
template<typename T>
[[nodiscard]] constexpr bool operator<=(nullptr_t, move_ptr<T> const& right) { return !(right < nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator>=(move_ptr<T> const& left, nullptr_t) { return !(left < nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator>=(nullptr_t, move_ptr<T> const& right) { return !(nullptr < right); }

template<typename T>
std::ostream& operator<<(std::ostream& os, move_ptr<T> const& ptr) {
    os << ptr;
    return os;
}

} // namespace cth::ptr



template<typename T>
struct std::hash<cth::move_ptr<T>> /* NOLINT(cert-dcl58-cpp) this is a valid overload for std::hash */ {
    std::size_t operator()(cth::move_ptr<T> const& ptr) const noexcept {
        using ptr_t = typename cth::move_ptr<T>::pointer;
        return std::hash<ptr_t>{}(ptr.get());
    }
};

namespace cth::ptr {
template<typename T>
struct const_ptr {
    using pointer = T const*;
    using element_type = T;

    constexpr const_ptr() noexcept = default;
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr const_ptr(T const* ptr) noexcept : _ptr(ptr) {}
    constexpr ~const_ptr() = default;

private:
    T const* _ptr = nullptr;

public:
    [[nodiscard]] constexpr T const* get() const { return _ptr; }
    [[nodiscard]] constexpr T const* operator->() const { return _ptr; }
    [[nodiscard]] constexpr T const& operator*() const { return *_ptr; }
    [[nodiscard]] explicit constexpr operator bool() const { return static_cast<bool>(_ptr); }

    const_ptr(const_ptr const& other) noexcept = default;
    const_ptr& operator=(const_ptr const& other) noexcept = default;

    const_ptr(const_ptr&& other) noexcept = default;
    const_ptr& operator=(const_ptr&& other) noexcept = default;
};

template<typename T, typename... Args>
[[nodiscard]] constexpr const_ptr<T> make_const(Args&&... args) { return const_ptr<T>(new T{std::forward<Args>(args)...}); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator==(const_ptr<T> const& ptr, const_ptr<U> const& other) { return ptr.get() == other.get(); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator!=(const_ptr<T> const& left, const_ptr<U> const& right) { return !(left.get() == right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<(const_ptr<T> const& left, const_ptr<U> const& right) {
    using left_t = typename const_ptr<T>::pointer;
    using right_t = typename const_ptr<U>::pointer;
    using ptr_t = std::common_type_t<left_t, right_t>;

    return std::less<ptr_t>{}(left.get(), right.get());
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>(const_ptr<T> const& left, const_ptr<U> const& right) { return right < left; }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<=(const_ptr<T> const& left, const_ptr<U> const& right) { return !(right < left); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>=(const_ptr<T> const& left, const_ptr<U> const& right) { return !(left < right); }

template<typename T>
[[nodiscard]] constexpr bool operator==(const_ptr<T> const& ptr, std::nullptr_t) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t, const_ptr<T> const& ptr) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(const_ptr<T> const& ptr, std::nullptr_t) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(std::nullptr_t, const_ptr<T> const& ptr) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator<(const_ptr<T> const& left, nullptr_t) {
    using ptr_t = typename const_ptr<T>::pointer;
    return std::less<ptr_t>{}(left.get(), nullptr);
}
template<typename T>
[[nodiscard]] constexpr bool operator<(nullptr_t, const_ptr<T> const& right) {
    using ptr_t = typename const_ptr<T>::pointer;
    return std::less<ptr_t>{}(nullptr, right.get());
}
template<typename T>
[[nodiscard]] constexpr bool operator>(const_ptr<T> const& left, nullptr_t) { return nullptr < left; }
template<typename T>
[[nodiscard]] constexpr bool operator>(nullptr_t, const_ptr<T> const& right) { return right < nullptr; }
template<typename T>
[[nodiscard]] constexpr bool operator<=(const_ptr<T> const& left, nullptr_t) { return !(nullptr < left); }
template<typename T>
[[nodiscard]] constexpr bool operator<=(nullptr_t, const_ptr<T> const& right) { return !(right < nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator>=(const_ptr<T> const& left, nullptr_t) { return !(left < nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator>=(nullptr_t, const_ptr<T> const& right) { return !(nullptr < right); }

template<typename T>
std::ostream& operator<<(std::ostream& os, const_ptr<T> const& ptr) {
    os << ptr;
    return os;
}
}



template<typename T>
struct std::hash<cth::ptr::const_ptr<T>> /* NOLINT(cert-dcl58-cpp) this is a valid overload for std::hash */ {
    std::size_t operator()(cth::ptr::const_ptr<T> const& ptr) const noexcept {
        using ptr_t = typename cth::ptr::const_ptr<T>::pointer const;
        return std::hash<ptr_t>{}(ptr.get());
    }
};
