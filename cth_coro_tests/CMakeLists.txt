cth_glob_cpp(CTH_CORO_TESTS_SRC "src")

add_executable(cth_coro_tests ${CTH_CORO_TESTS_SRC})

cth_target_enable_build_cache(cth_coro_tests)

target_include_directories(
        cth_coro_tests
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(cth_coro_tests PRIVATE GTest::gtest_main cth::coro cth::cth)

include(GoogleTest)
gtest_discover_tests(cth_coro_tests)