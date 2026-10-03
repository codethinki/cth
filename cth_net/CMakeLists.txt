cth_glob_cpp(CTH_NET_SRC "src")
cth_glob_cpp(CTH_NET_INCL "incl")

add_library(cth_net STATIC ${CTH_NET_SRC} ${CTH_NET_INCL})
add_library(cth::net ALIAS cth_net)

cth_target_enable_build_cache(cth_net OPTIONAL)

cth_pkg_target_include_directories(
        cth_net
        PUBLIC incl
)

cth_pkg_target_find_package(cth_net libhv CONFIG REQUIRED)
# vcpkg's libhv exports hv for shared and hv_static for static builds
cth_target_alias_any(libhv::hv hv hv_static)

target_link_libraries(cth_net PUBLIC cth::cth cth::os)
target_link_libraries(cth_net PRIVATE libhv::hv)

if(WIN32)
    target_compile_definitions(cth_net PRIVATE _WIN32_WINNT=0x0A00)
endif()

if(CTH_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
