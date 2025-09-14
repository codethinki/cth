glob_cpp(TESTS_SRC "src")

add_executable(tests ${TESTS_SRC})

target_include_directories(
        tests
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(tests PRIVATE GTest::gtest_main cth cth::win)

include(GoogleTest)
gtest_discover_tests(tests)