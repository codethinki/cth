cmake_minimum_required(VERSION 3.21)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


include(glob_cpp)
include(add_resources)
include(install_utility)


project(cth LANGUAGES CXX VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



#exported
add_subdirectory(cth)
add_subdirectory(cth_windows)

#test
enable_testing()
add_subdirectory(cth_test)
add_subdirectory(cth_integration_test)

#install
setup_install()

