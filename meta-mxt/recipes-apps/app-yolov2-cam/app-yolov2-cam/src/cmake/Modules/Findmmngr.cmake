#[=======================================================================[.rst:
Findmmngr
------------

Find mmngr headers and library.

Imported Targets
^^^^^^^^^^^^^^^^

``mmngr::mmngr``
  The mmngr library, if found.
``mmngr::mmngrbuf``
  The mmngrbuf library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables in your project:

``mmngr_FOUND``
  true if (the requested version of) mmngr is available.
``mmngr_LIBRARIES``
  the libraries to link against to use mmngr.
``mmngr_INCLUDE_DIRS``
  where to find the mmngr headers.

#]=======================================================================]

# Find paths to header files and library files
find_path(mmngr_INCLUDE_DIR
    NAMES
        mmngr_buf_user_public.h
)  
find_library(mmngr_LIBRARY 
    NAMES 
        mmngr
)
find_library(mmngrbuf_LIBRARY 
    NAMES 
        mmngrbuf
)
mark_as_advanced(mmngr_INCLUDE_DIR mmngr_LIBRARY mmngrbuf_LIBRARY)

# Use the package standard args handler module to handle commonly exported variables. 
# In this case, it will set mmngr_FOUND if the required args are set.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mmngr 
    FOUND_VAR
        mmngr_FOUND
    REQUIRED_VARS 
        mmngr_LIBRARY 
        mmngrbuf_LIBRARY
        mmngr_INCLUDE_DIR
)

find_package_handle_standard_args(mmngrbuf 
    FOUND_VAR
        mmngrbuf_FOUND
    REQUIRED_VARS 
        mmngrbuf_LIBRARY
        mmngr_INCLUDE_DIR
)
# Create an imported target named mmngr::mmngr
# Note that target type is UNKNOWN because we don't 
# whether the user will build it as a SHARED or STATIC library.
# Must set target properties so that they are transitively passed to targets which link to this one.
if(mmngr_FOUND AND NOT TARGET mmngr::mmngr)
    add_library(mmngr::mmngr UNKNOWN IMPORTED)
    set_target_properties(mmngr::mmngr 
        PROPERTIES 
            IMPORTED_LOCATION ${mmngr_LIBRARY}
    )
    target_include_directories(mmngr::mmngr 
        INTERFACE 
          ${mmngr_INCLUDE_DIR})
endif()

if(mmngrbuf_FOUND AND NOT TARGET mmngr::mmngrbuf)
    add_library(mmngr::mmngrbuf UNKNOWN IMPORTED)
    set_target_properties(mmngr::mmngrbuf 
        PROPERTIES 
            IMPORTED_LOCATION ${mmngrbuf_LIBRARY}
    )
    target_include_directories(mmngr::mmngrbuf 
        INTERFACE 
          ${mmngr_INCLUDE_DIR})
endif()

# For people using old CMake style we must set some commonly used build variables.
if(mmngr_FOUND)
  set(mmngr_LIBRARIES ${mmngr_LIBRARY} ${mmngrbuf_LIBRARY})
  set(mmngr_INCLUDE_DIRS ${mmngr_INCLUDE_DIR})
endif()
