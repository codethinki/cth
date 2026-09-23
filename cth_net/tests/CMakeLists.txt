cth_glob_cpp(CTH_NET_TESTS_SRC "src")

add_executable(cth_net_tests ${CTH_NET_TESTS_SRC})

cth_target_enable_build_cache(cth_net_tests OPTIONAL)

target_include_directories(
        cth_net_tests
        PRIVATE src
)

find_package(GTest CONFIG REQUIRED)

target_link_libraries(cth_net_tests PRIVATE GTest::gtest_main cth::net cth::coro hv)

if(WIN32)
    target_compile_definitions(cth_net_tests PRIVATE _WIN32_WINNT=0x0A00)
endif()

include(GoogleTest)
gtest_discover_tests(cth_net_tests)
