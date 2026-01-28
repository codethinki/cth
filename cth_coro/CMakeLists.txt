cth_glob_cpp(CTH_CORO_SRC "src")
cth_glob_cpp(CTH_CORO_INCL "incl")

add_library(cth_coro STATIC ${CTH_CORO_SRC} ${CTH_CORO_INCL})
add_library(cth::coro ALIAS cth_coro)

cth_target_enable_build_cache(cth_coro)

cth_pkg_target_include_directories(
        cth_coro
        PUBLIC incl
)

cth_pkg_target_find_package(cth_coro Boost CONFIG REQUIRED COMPONENTS asio)

target_link_libraries(cth_coro PUBLIC cth::cth)

target_link_libraries(cth_coro PRIVATE Boost::asio)

if(WIN32)
    target_link_libraries(cth_coro PRIVATE cth::win)
endif()
    