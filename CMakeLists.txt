cmake_minimum_required(VERSION 4.1.1)


# In 2026, you likely need this specific UUID for Clang-cl module support
project(cth LANGUAGES CXX VERSION 1.0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(glob_cpp)
include(add_resources)
include(install_utility)
include(set_compiler_specifics)

if(MSVC)
	set(CTH_CXX_STANDARD 23)
else()
	set(CTH_CXX_STANDARD 26)
endif()

set(CMAKE_CXX_STANDARD ${CTH_CXX_STANDARD})
set(CMAKE_C_STANDARD ${CTH_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED ON)



set_compiler_specifics()


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
