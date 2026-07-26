cth_glob_cpp(CTH_OS_TESTS_SRC "src")

add_executable(cth_os_tests ${CTH_OS_TESTS_SRC})

cth_target_enable_build_cache(cth_os_tests OPTIONAL)

target_include_directories(
        cth_os_tests
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(cth_os_tests PRIVATE GTest::gtest_main cth::os cth::cth)

include(GoogleTest)
gtest_discover_tests(cth_os_tests)
