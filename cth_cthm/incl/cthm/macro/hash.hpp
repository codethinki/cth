#pragma once

import cth.hash;

#define CTH_DEV_HASH_OVERLOAD(T, func, prefix)                                                               \
    export template<>                                                                                        \
    struct std::hash<T> {                                                                                    \
        prefix size_t                                                                                        \
        operator()(T const& x) const noexcept(cth::mta::nothrow_callable_with<func, T const&>) {             \
            static_assert(                                                                                   \
                cth::mta::call_signature<func, size_t, T const&>,                                            \
                "invalid hash overload function: " #func " for " #T                                          \
            );                                                                                               \
            return func(x);                                                                                  \
        }                                                                                                    \
    };

#define CTH_HASH_OVERLOAD(T, func) CTH_DEV_HASH_OVERLOAD(T, func, )
#define CTH_CXPR_HASH_OVERLOAD(T, func) CTH_DEV_HASH_OVERLOAD(T, func, constexpr)

#define CTH_HASH_AGGREGATE(type)                                                                             \
    export template<>                                                                                        \
    struct std::hash<type> {                                                                                 \
        cxpr size_t operator()(type const& x) const noexcept { return cth::hash::hash_aggregate(x); }        \
    };
