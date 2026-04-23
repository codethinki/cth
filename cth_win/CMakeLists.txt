cth_glob_cpp(CTH_WIN_SRC "src")
cth_glob_cpp(CTH_WIN_INCL "incl")

add_library(cth_win STATIC ${CTH_WIN_SRC} ${CTH_WIN_INCL})
add_library(cth::win ALIAS cth_win)

cth_target_enable_build_cache(cth_win OPTIONAL)

cth_pkg_target_include_directories(
        cth_win
        PUBLIC incl
        PRIVATE src
)

target_link_libraries(cth_win PUBLIC cth::cth)

# tests
if(CTH_ENABLE_TESTS)
    add_subdirectory(tests)
endif()