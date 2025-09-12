cmake_minimum_required(VERSION 3.21)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(glob_cpp)
include(target_install_include_directories)
include(add_resources)

project(cth LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


enable_testing()

# Add subprojects
add_subdirectory(cth)
add_subdirectory(cth_test)
add_subdirectory(cth_integration_test)
add_subdirectory(cth_windows)
