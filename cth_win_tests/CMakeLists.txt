cth_glob_cpp(CTH_WIN_TESTS_SRC "src")

add_executable(cth_win_tests ${CTH_WIN_TESTS_SRC})
cth_add_resources(cth_win_tests "res")


target_include_directories(
        cth_win_tests
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(cth_win_tests PRIVATE GTest::gtest_main cth::win cth::cth)

include(GoogleTest)
gtest_discover_tests(cth_win_tests)
