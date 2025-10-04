#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cth::cth" for configuration "Release"
set_property(TARGET cth::cth APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cth::cth PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/cth.lib"
  )

list(APPEND _cmake_import_check_targets cth::cth )
list(APPEND _cmake_import_check_files_for_cth::cth "${_IMPORT_PREFIX}/lib/cth.lib" )

# Import target "cth::win" for configuration "Release"
set_property(TARGET cth::win APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(cth::win PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/cth_win.lib"
  )

list(APPEND _cmake_import_check_targets cth::win )
list(APPEND _cmake_import_check_files_for_cth::win "${_IMPORT_PREFIX}/lib/cth_win.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
