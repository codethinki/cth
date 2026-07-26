cth_glob_cpp(CTH_OS_SRC "src")
cth_glob_cpp(CTH_OS_INCL "incl")

add_library(cth_os STATIC ${CTH_OS_SRC} ${CTH_OS_INCL})
add_library(cth::os ALIAS cth_os)

cth_target_enable_build_cache(cth_os OPTIONAL)

cth_pkg_target_include_directories(
        cth_os
        PUBLIC incl
        PRIVATE src
)

target_link_libraries(cth_os PUBLIC cth::cth)

# tests
if(CTH_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
