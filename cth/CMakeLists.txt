cth_glob_cpp(CTH_SRC "src")
cth_glob_cpp(CTH_INCL "incl")

add_library(cth INTERFACE)
add_library(cth::cth ALIAS cth)

cth_pkg_target_include_directories(
        cth
        INTERFACE incl
)

target_compile_options(cth INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)

# cth/hash/hash_aggregates.hpp includes boost/pfr.hpp
cth_pkg_target_find_package(cth boost_pfr CONFIG REQUIRED)
target_link_libraries(cth INTERFACE Boost::pfr)

# cth/data/hash_map.hpp includes gtl/phmap.hpp, gtl installs no cmake package
cth_pkg_add_find_modules(cmake/Findgtl.cmake)
cth_pkg_target_find_package(cth gtl MODULE REQUIRED)
target_link_libraries(cth INTERFACE gtl::gtl)

target_link_libraries(cth INTERFACE $<$<AND:$<CXX_COMPILER_ID:GNU,Clang>,$<NOT:$<PLATFORM_ID:Windows>>>:stdc++exp>)

# tests
if(CTH_ENABLE_TESTS)
    add_subdirectory(tests)
endif()