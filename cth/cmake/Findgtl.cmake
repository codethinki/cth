# gtl installs only its headers and no cmake package, this module creates the target gtl::gtl
include(FindPackageHandleStandardArgs)

find_path(gtl_INCLUDE_DIR gtl/phmap.hpp)
find_package_handle_standard_args(gtl REQUIRED_VARS gtl_INCLUDE_DIR)

if(gtl_FOUND AND NOT TARGET gtl::gtl)
    add_library(gtl::gtl INTERFACE IMPORTED)
    target_include_directories(gtl::gtl SYSTEM INTERFACE "${gtl_INCLUDE_DIR}")
endif()

mark_as_advanced(gtl_INCLUDE_DIR)
