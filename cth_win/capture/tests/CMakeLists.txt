cth_glob_cpp(CTH_WIN_CAPTURE_TESTS_SRC "src")

add_executable(cth_win_capture_tests ${CTH_WIN_CAPTURE_TESTS_SRC})

cth_target_enable_build_cache(cth_win_capture_tests OPTIONAL)

target_include_directories(
        cth_win_capture_tests
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(cth_win_capture_tests PRIVATE GTest::gtest_main cth::win::capture cth::win)

include(GoogleTest)
gtest_discover_tests(cth_win_capture_tests)
