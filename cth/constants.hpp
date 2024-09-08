#pragma once
#ifdef _DEBUG
#ifndef CTH_RELEASE_MODE
#ifndef CTH_DEBUG_MODE
#define CTH_DEBUG_MODE
#endif
#endif
#elifdef _RELEASE
#ifndef CTH_DEBUG_MODE
#ifndef CTH_RELEASE_MODE
#define CTH_RELEASE_MODE
#endif
#endif
#endif

#ifdef CTH_DEBUG_MODE
#ifdef CTH_RELEASE_MODE
#error "CTH_DEBUG_MODE and CTH_RELEASE_MODE must not be defined at the same time."
#endif
#endif


#ifndef CTH_DEBUG_MODE
#ifndef CTH_RELEASE_MODE
#warning "Neither CTH_DEBUG_MODE nor CTH_RELEASE_MODE is defined. Defaulting to CTH_DEBUG_MODE."
#define CTH_DEBUG_MODE
#endif
#endif



#ifdef CTH_DEBUG_MODE
#define CTH_DEBUG_IMPL
#define CTH_DEBUG_INLINE_IMPL(code) {code}

#define CTH_RELEASE_NOEXCEPT noexcept(false)
#define CTH_RELEASE_CONSTEXPR
#else
#define CTH_DEBUG_IMPL = default;
#define CTH_DEBUG_INLINE_IMPL(code) {}

#define CTH_RELEASE_NOEXCEPT noexcept(true)
#define CTH_RELEASE_CONSTEXPR constexpr
#endif
#include <cstdint>



namespace cth {
enum CompilationMode : uint8_t {
    MODE_DEBUG,
    MODE_RELEASE,
};

constexpr auto COMPILATION_MODE =
#ifdef CTH_DEBUG_MODE
    MODE_DEBUG;
#else
    MODE_RELEASE;
#endif

}
