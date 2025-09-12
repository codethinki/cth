project(cth_test LANGUAGES CXX)

glob_cpp(CTH_TEST_SRC "src")

add_executable(cth_test ${CTH_TEST_SRC})

target_include_directories(
        cth_test
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(cth_test PRIVATE GTest::gtest_main cth)

include(GoogleTest)
gtest_discover_tests(cth_test)