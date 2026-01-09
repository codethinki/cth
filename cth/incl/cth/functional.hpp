#pragma once
#include "cth/meta/concepts.hpp"

#include <functional>


namespace cth::fn {

/**
 * converts a member function to a lambda
 * @tparam Fn function to call
 * @param obj object instance to call function on
 */
template<auto Fn, class T>
auto to_lambda(T& obj) {
    return [&obj]<class... Args>(Args&&... args) {
        static_assert(cth::mta::member_callable_with<T, Fn, Args...>, "Fn must be callable on T");
        return std::invoke(Fn, obj, std::forward<Args>(args)...);
    };
}

}
