cmake_minimum_required(VERSION 3.23)

project(cth LANGUAGES CXX VERSION 1.0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


include(glob_cpp)
include(add_resources)
include(install_utility)
include(set_compiler_specifics)

set_compiler_specifics()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#exported
add_subdirectory(cth)
add_subdirectory(cth_win)
add_subdirectory(cth_coro)

create_package()


#test
enable_testing()
add_subdirectory(cth_tests)
add_subdirectory(cth_coro_tests)
add_subdirectory(cth_win_tests)


add_subdirectory(integration_tests)

#playground
add_subdirectory(playground)
