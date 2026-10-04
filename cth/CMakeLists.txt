cth_glob_cpp(CTH_SRC "src")
cth_glob_cpp(CTH_INCL "incl")

add_library(cth INTERFACE)
add_library(cth::cth ALIAS cth)

cth_pkg_target_include_directories(
        cth
        INTERFACE incl
)

target_compile_options(cth INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)

target_link_libraries(cth INTERFACE $<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<NOT:$<PLATFORM_ID:Windows>>>:stdc++exp>)

# tests
if(CTH_ENABLE_TESTS)
    add_subdirectory(tests)
endif()