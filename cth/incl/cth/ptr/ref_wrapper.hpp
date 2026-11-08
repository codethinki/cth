#pragma once
#include <functional>
#include <memory>


namespace cth {
/**
 * `std::reference_wrapper` with `->` and `*`
 * @details
 * - does not own the object, never refers to nothing
 * - takes everything that converts to `T&`: `T&`, a derived type, a type with a conversion operator
 * - assignment rebinds the reference
 * - `==` compares the objects, like `std::reference_wrapper`
 * @tparam T type of the object
 */
template<class T>
class ref_wrapper : public std::reference_wrapper<T> {
public:
    using std::reference_wrapper<T>::reference_wrapper;

    [[nodiscard]] constexpr T* operator->() const noexcept { return std::addressof(this->get()); }
    [[nodiscard]] constexpr T& operator*() const noexcept { return this->get(); }
};

template<class T>
ref_wrapper(T&) -> ref_wrapper<T>;
}
