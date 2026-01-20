cth_glob_cpp(CTH_TESTS_SRC "src")

add_executable(cth_tests ${CTH_TESTS_SRC})

cth_target_enable_build_cache(cth_tests)

target_include_directories(
        cth_tests
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(cth_tests PRIVATE GTest::gtest_main cth::cth)

include(GoogleTest)
gtest_discover_tests(cth_tests)