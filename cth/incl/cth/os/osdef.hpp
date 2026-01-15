#pragma once


#if defined(_WIN32) && !defined(CTH_PLATFORM_WINDOWS)
#define CTH_PLATFORM_WINDOWS
#endif

#if defined(__APPLE__)
#define CTH_PLATFORM_MACOS
#endif

#if defined(__linux__) && !defined(CTH_PLATFORM_LINUX)
#define CTH_PLATFORM_LINUX
#endif

#if defined(__ANDROID__) && !defined(CTH_PLATFORM_ANDROID)
#define CTH_PLATFORM_ANDROID
#endif

#if defined(CTH_PLATFORM_WINDOWS) + defined(CTH_PLATFORM_LINUX) + defined(CTH_PLATFORM_ANDROID) + defined(CTH_PLATFORM_MACOS) != 1
#error "multiple or no platforms defined"
#endif


#ifdef CTH_PLATFORM_WINDOWS
#define CTH_FS_WINDOWS
#elif defined(CTH_PLATFORM_LINUX) || defined(CTH_PLATFORM_ANDROID) || defined(CTH_PLATFORM_MACOS)
#define CTH_FS_POSIX
#endif
