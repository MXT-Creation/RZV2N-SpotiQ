#[=======================================================================[.rst:
Findv4l2subdev
------------

Find v4l2subdev headers and library.

Imported Targets
^^^^^^^^^^^^^^^^

``v4l2subdev::v4l2subdev``
  The v4l2subdev library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``v4l2subdev_FOUND``
  true if (the requested version of) v4l2subdev is available.
``v4l2subdev_LIBRARIES``
  the libraries to link against to use v4l2subdev.
``v4l2subdev_INCLUDE_DIRS``
  where to find the v4l2subdev headers.

#]=======================================================================]

# Find paths to header files and library files
find_path(v4l2subdev_INCLUDE_DIR
    NAMES
        v4l2subdev.h
    HINTS 
        /usr/include/mediactl
)  
find_library(v4l2subdev_LIBRARY 
    NAMES 
        v4l2subdev
)

mark_as_advanced(v4l2subdev_INCLUDE_DIR v4l2subdev_LIBRARY)

# Use the package standard args handler module to handle commonly exported variables. 
# In this case, it will set v4l2subdev_FOUND if the required args are set.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(v4l2subdev 
    FOUND_VAR
        v4l2subdev_FOUND
    REQUIRED_VARS 
        v4l2subdev_LIBRARY 
        v4l2subdev_INCLUDE_DIR
)


# Create an imported target named v4l2subdev::v4l2subdev
# Note that target type is UNKNOWN because we don't 
# whether the user will build it as a SHARED or STATIC library.
# Must set target properties so that they are transitively passed to targets which link to this one.
if(v4l2subdev_FOUND AND NOT TARGET v4l2subdev::v4l2subdev)
    add_library(v4l2subdev::v4l2subdev UNKNOWN IMPORTED)
    set_target_properties(v4l2subdev::v4l2subdev 
        PROPERTIES 
            IMPORTED_LOCATION ${v4l2subdev_LIBRARY}
    )
    target_include_directories(v4l2subdev::v4l2subdev 
        INTERFACE 
          ${v4l2subdev_INCLUDE_DIR})
endif()

# For people using old CMake style we must set some commonly used build variables.
if(v4l2subdev_FOUND)
  set(v4l2subdev_LIBRARIES ${v4l2subdev_LIBRARY})
  set(v4l2subdev_INCLUDE_DIRS ${v4l2subdev_INCLUDE_DIR})
endif()
