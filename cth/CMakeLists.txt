cth_glob_cpp(CTH_SRC "src")
cth_glob_cpp(CTH_INCL "incl")

add_library(cth INTERFACE)
add_library(cth::cth ALIAS cth)

cth_pkg_target_include_directories(
        cth
        INTERFACE incl
)

# tests
if(CTH_ENABLE_TESTS)
    add_subdirectory(tests)
endif()