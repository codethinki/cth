cth_glob_cppm(CTHM_MODULES "modules")

add_library(cth_cthm STATIC)
add_library(cth::cthm ALIAS cth_cthm)

cth_pkg_target_include_directories(
    cth_cthm
    PUBLIC modules incl
)

cth_pkg_target_add_modules(
        cth_cthm
        PUBLIC ${CTHM_MODULES}
)

cth_pkg_target_find_package(cth_cthm Boost CONFIG REQUIRED COMPONENTS pfr)

target_link_libraries(cth_cthm PRIVATE Boost::pfr)

