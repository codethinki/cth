cmake_minimum_required(VERSION 4.1.1)


# In 2026, you likely need this specific UUID for Clang-cl module support
project(cth LANGUAGES CXX VERSION 1.0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(setup_utilities)
include(target_utilities)
include(install_utilities)

set_compiler_specifics()
set_cxx_standard()







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
