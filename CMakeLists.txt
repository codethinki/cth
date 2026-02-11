cmake_minimum_required(VERSION 4.1.1)
project(
  cth
  LANGUAGES CXX
  VERSION 1.0.0)

include(cth_setup_utilities)
include(cth_target_utilities)
include(cth_install_utilities)

cth_set_compiler_specifics()
cth_set_latest_c_cpp_standard()

# exported
add_subdirectory(cth)
add_subdirectory(cth_win)
add_subdirectory(cth_coro)
add_subdirectory(cth_cthm)

cth_create_package()

# test
enable_testing()
add_subdirectory(cth_tests)
add_subdirectory(cth_coro_tests)
add_subdirectory(cth_win_tests)

add_subdirectory(integration_tests)
 
# formatting

cth_glob_cpp(
  FORMAT_FILES
  "cth"
  "cth_coro"
  "cth_coro_tests"
  "cth_cthm"
  "cth_tests"
  "cth_win"
  "cth_win_tests"
  "integration_tests")
cth_add_clang_format_target(clang-format ${FORMAT_FILES})

# playground
add_subdirectory(playground)
