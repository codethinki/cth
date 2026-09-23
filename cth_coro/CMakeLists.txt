cth_glob_cpp(CTH_CORO_SRC "src")
cth_glob_cpp(CTH_CORO_INCL "incl")

add_library(cth_coro STATIC ${CTH_CORO_SRC} ${CTH_CORO_INCL})
add_library(cth::coro ALIAS cth_coro)

cth_target_enable_build_cache(cth_coro OPTIONAL)

cth_pkg_target_include_directories(
        cth_coro
        PUBLIC incl
)

cth_pkg_target_find_package(cth_coro asio CONFIG REQUIRED)

target_link_libraries(cth_coro PUBLIC cth::cth cth::os)

target_link_libraries(cth_coro PRIVATE asio::asio)

if(WIN32)
    target_compile_definitions(cth_coro PRIVATE _WIN32_WINNT=0x0A00)
endif()

# tests
if(CTH_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
