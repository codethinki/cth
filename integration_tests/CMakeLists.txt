cth_glob_cpp(INTEGRATION_TESTS_SRC "src")

add_executable(integration_tests ${INTEGRATION_TESTS_SRC})
cth_target_enable_build_cache(integration_tests)

cth_add_resources(integration_tests "res")

target_include_directories(
        integration_tests
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(integration_tests PRIVATE GTest::gtest_main cth cth::win)

include(GoogleTest)
gtest_discover_tests(integration_tests)