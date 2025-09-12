project(cth_integration_test LANGUAGES CXX)

glob_cpp(CTH_INTEGRATION_TEST_SRC "src")

add_executable(cth_integration_test ${CTH_INTEGRATION_TEST_SRC})
add_resources(cth_integration_test "res")

target_include_directories(
        cth_integration_test
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(cth_integration_test PRIVATE GTest::gtest_main cth)

include(GoogleTest)
gtest_discover_tests(cth_integration_test)