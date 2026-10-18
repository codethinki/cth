cmake_minimum_required(VERSION 4.1.1)
project(
  cth
  LANGUAGES CXX
  VERSION 1.0.0
)

include(cth_setup_utilities)
include(cth_target_utilities)
include(cth_install_utilities)

cth_set_compiler_specifics()
cth_set_latest_c_cpp_standard()
set(CMAKE_CXX_SCAN_FOR_MODULES FALSE)

if(CTH_ENABLE_TESTS)
	enable_testing()
endif()

# exported, a parent project turns components off with set(CTH_BUILD_<COMPONENT> OFF)
add_subdirectory(cth)
cth_add_subdirectory(cth_os CTH_BUILD_OS)
cth_add_subdirectory(cth_coro CTH_BUILD_CORO)
cth_add_subdirectory(cth_net CTH_BUILD_NET)
cth_add_subdirectory(cth_win CTH_BUILD_WIN WIN32)

cth_create_package()

# test
if(CTH_ENABLE_TESTS)
	add_subdirectory(integration_tests)
endif()

# formatting
cth_glob_cpp(
  FORMAT_FILES
  "cth"
  "cth_coro"
  "cth_net"
  "cth_os"
  "cth_win"
  "integration_tests"
  "playground"
)

#cth_add_uncrustify_target(uncrustify-format OPTIONAL ${FORMAT_FILES})

