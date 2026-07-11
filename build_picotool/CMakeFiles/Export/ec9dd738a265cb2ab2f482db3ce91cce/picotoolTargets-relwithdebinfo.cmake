#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "picotool" for configuration "RelWithDebInfo"
set_property(TARGET picotool APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(picotool PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/picotool.exe"
  )

list(APPEND _cmake_import_check_targets picotool )
list(APPEND _cmake_import_check_files_for_picotool "${_IMPORT_PREFIX}/bin/picotool.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
