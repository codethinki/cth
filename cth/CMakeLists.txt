cth_glob_cpp(CTH_SRC "src")
cth_glob_cpp(CTH_INCL "incl")

add_library(cth INTERFACE)
add_library(cth::cth ALIAS cth)

cth_pkg_target_include_directories(
        cth
        INTERFACE incl
)

# the logging macros rely on __VA_OPT__, which needs MSVC's conforming preprocessor
target_compile_options(cth INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)

# libstdc++ ships std::stacktrace in a separate library (the MSVC STL has it built in)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
    target_link_libraries(cth INTERFACE stdc++exp)
endif()

# tests
if(CTH_ENABLE_TESTS)
    add_subdirectory(tests)
endif()