#pragma once
#include <memory>
#include <type_traits>


namespace cth::mem {
/**
 * \brief move aware pointer (nullptr after move)
 * \note NOT memory safe
 */
template<typename T>
struct basic_ptr {
    using pointer = T*;
    using element_type = T;
    constexpr basic_ptr() noexcept = default;
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr basic_ptr(T* ptr) : _ptr(ptr) {}
    constexpr ~basic_ptr() = default;

private:
    T* _ptr = nullptr;

public:
    constexpr void free() noexcept {
        delete _ptr;
        _ptr = nullptr;
    }

    constexpr void swap(basic_ptr& other) noexcept { std::swap(_ptr, other._ptr); }

    [[nodiscard]] constexpr T* get() const { return _ptr; }

    [[nodiscard]] constexpr T* release() noexcept {
        auto ptr = _ptr;
        _ptr = nullptr;
        return ptr;
    }
    [[nodiscard]] constexpr T* operator->() const { return _ptr; }
    [[nodiscard]] constexpr T& operator*() const { return *_ptr; }
    [[nodiscard]] explicit constexpr operator bool() const { return static_cast<bool>(_ptr); }

    basic_ptr(const basic_ptr& other) noexcept = default;
    basic_ptr& operator=(const basic_ptr& other) noexcept = default;

    basic_ptr(basic_ptr&& other) noexcept : _ptr(other.release()) {}
    basic_ptr& operator=(basic_ptr&& other) noexcept {
        _ptr = other.release();
        return *this;
    }


};
template<typename T>
constexpr void swap(basic_ptr<T>& left, basic_ptr<T>& right) noexcept { left.swap(right); }

template<typename T, typename... Args>
[[nodiscard]] constexpr basic_ptr<T> make_basic(Args&&... args) { return basic_ptr<T>(new T(std::forward<Args>(args)...)); }

template<typename T, typename U>
[[nodiscard]] constexpr bool operator==(const basic_ptr<T>& ptr, const basic_ptr<U>& other) { return ptr.get() == other.get(); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator!=(const basic_ptr<T>& left, const basic_ptr<U>& right) { return !(left.get() == right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<(const basic_ptr<T>& left, const basic_ptr<U>& right) {
    using left_t = typename basic_ptr<T>::pointer;
    using right_t = typename basic_ptr<U>::pointer;
    using ptr_t = std::common_type_t<left_t, right_t>;

    return std::less<ptr_t>{}(left.get(), right.get());
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>(const basic_ptr<T>& left, const basic_ptr<U>& right) { return right < left; }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<=(const basic_ptr<T>& left, const basic_ptr<U>& right) { return !(right < left); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>=(const basic_ptr<T>& left, const basic_ptr<U>& right) { return !(left < right); }

template<typename T>
[[nodiscard]] constexpr bool operator==(const basic_ptr<T>& ptr, std::nullptr_t) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t, const basic_ptr<T>& ptr) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(const basic_ptr<T>& ptr, std::nullptr_t) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(std::nullptr_t, const basic_ptr<T>& ptr) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator<(const basic_ptr<T>& left, nullptr_t) {
    using ptr_t = typename basic_ptr<T>::pointer;
    return std::less<ptr_t>{}(left.get(), nullptr);
}
template<typename T>
[[nodiscard]] constexpr bool operator<(nullptr_t, const basic_ptr<T>& right) {
    using ptr_t = typename basic_ptr<T>::pointer;
    return std::less<ptr_t>{}(nullptr, right.get());
}
template<typename T>
[[nodiscard]] constexpr bool operator>(const basic_ptr<T>& left, nullptr_t) { return nullptr < left; }
template<typename T>
[[nodiscard]] constexpr bool operator>(nullptr_t, const basic_ptr<T>& right) { return right < nullptr; }
template<typename T>
[[nodiscard]] constexpr bool operator<=(const basic_ptr<T>& left, nullptr_t) { return !(nullptr < left); }
template<typename T>
[[nodiscard]] constexpr bool operator<=(nullptr_t, const basic_ptr<T>& right) { return !(right < nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator>=(const basic_ptr<T>& left, nullptr_t) { return !(left < nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator>=(nullptr_t, const basic_ptr<T>& right) { return !(nullptr < right); }

template<typename T>
std::ostream& operator<<(std::ostream& os, const basic_ptr<T>& ptr) {
    os << ptr;
    return os;
}

} // namespace cth::mem



template<typename T>
struct std::hash<cth::mem::basic_ptr<T>> /* NOLINT(cert-dcl58-cpp) this is a valid overload for std::hash */ {
    std::size_t operator()(const cth::mem::basic_ptr<T>& ptr) const noexcept {
        using ptr_t = typename cth::mem::basic_ptr<T>::pointer;
        return std::hash<ptr_t>{}(ptr.get());
    }
};

namespace cth::mem {
template<typename T>
struct const_ptr {
    using pointer = const T*;
    using element_type = T;

    constexpr const_ptr() noexcept = default;
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr const_ptr(const T* ptr) noexcept : _ptr(ptr) {}
    constexpr ~const_ptr() = default;

private:
    const T* _ptr = nullptr;

public:
    [[nodiscard]] constexpr const T* get() const { return _ptr; }
    [[nodiscard]] constexpr const T* operator->() const { return _ptr; }
    [[nodiscard]] constexpr const T& operator*() const { return *_ptr; }
    [[nodiscard]] explicit constexpr operator bool() const { return static_cast<bool>(_ptr); }

    const_ptr(const const_ptr& other) noexcept = default;
    const_ptr& operator=(const const_ptr& other) noexcept = default;

    const_ptr(const_ptr&& other) noexcept = default;
    const_ptr& operator=(const_ptr&& other) noexcept = default;
};

template<typename T, typename... Args>
[[nodiscard]] constexpr const_ptr<T> make_const(Args&&... args) { return const_ptr<T>(new T{std::forward<Args>(args)...}); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator==(const const_ptr<T>& ptr, const const_ptr<U>& other) { return ptr.get() == other.get(); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator!=(const const_ptr<T>& left, const const_ptr<U>& right) { return !(left.get() == right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<(const const_ptr<T>& left, const const_ptr<U>& right) {
    using left_t = typename const_ptr<T>::pointer;
    using right_t = typename const_ptr<U>::pointer;
    using ptr_t = std::common_type_t<left_t, right_t>;

    return std::less<ptr_t>{}(left.get(), right.get());
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>(const const_ptr<T>& left, const const_ptr<U>& right) { return right < left; }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<=(const const_ptr<T>& left, const const_ptr<U>& right) { return !(right < left); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>=(const const_ptr<T>& left, const const_ptr<U>& right) { return !(left < right); }

template<typename T>
[[nodiscard]] constexpr bool operator==(const const_ptr<T>& ptr, std::nullptr_t) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t, const const_ptr<T>& ptr) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(const const_ptr<T>& ptr, std::nullptr_t) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(std::nullptr_t, const const_ptr<T>& ptr) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator<(const const_ptr<T>& left, nullptr_t) {
    using ptr_t = typename const_ptr<T>::pointer;
    return std::less<ptr_t>{}(left.get(), nullptr);
}
template<typename T>
[[nodiscard]] constexpr bool operator<(nullptr_t, const const_ptr<T>& right) {
    using ptr_t = typename const_ptr<T>::pointer;
    return std::less<ptr_t>{}(nullptr, right.get());
}
template<typename T>
[[nodiscard]] constexpr bool operator>(const const_ptr<T>& left, nullptr_t) { return nullptr < left; }
template<typename T>
[[nodiscard]] constexpr bool operator>(nullptr_t, const const_ptr<T>& right) { return right < nullptr; }
template<typename T>
[[nodiscard]] constexpr bool operator<=(const const_ptr<T>& left, nullptr_t) { return !(nullptr < left); }
template<typename T>
[[nodiscard]] constexpr bool operator<=(nullptr_t, const const_ptr<T>& right) { return !(right < nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator>=(const const_ptr<T>& left, nullptr_t) { return !(left < nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator>=(nullptr_t, const const_ptr<T>& right) { return !(nullptr < right); }

template<typename T>
std::ostream& operator<<(std::ostream& os, const const_ptr<T>& ptr) {
    os << ptr;
    return os;
}
}



template<typename T>
struct std::hash<cth::mem::const_ptr<T>> /* NOLINT(cert-dcl58-cpp) this is a valid overload for std::hash */ {
    std::size_t operator()(const cth::mem::const_ptr<T>& ptr) const noexcept {
        using ptr_t = const typename cth::mem::const_ptr<T>::pointer;
        return std::hash<ptr_t>{}(ptr.get());
    }
};
