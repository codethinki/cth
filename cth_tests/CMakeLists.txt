glob_cpp(CTH_TESTS_SRC "src")

add_executable(cth_tests ${CTH_TESTS_SRC})

target_include_directories(
        cth_tests
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(cth_tests PRIVATE GTest::gtest_main cth::cth)

include(GoogleTest)
gtest_discover_tests(cth_tests)