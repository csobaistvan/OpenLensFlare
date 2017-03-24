#=============================================================================
#
# FindOpenLensFlare - attempts to locate the OpenLensFlare library.
#
# This module defines the following variables (on success):
#
#   OpenLensFlare_INCLUDE_DIRS - where OpenLensFlare.h is located
#   OpenLensFlare_LIBRARIES - libraries to link against
#   OpenLensFlare_FOUND - if the library was found
#
# It is trying a few standard installation locations, but can be customized
# with the following variable:
#
#   OpenLensFlare_ROOT_DIR - root directory of an OpenLensFlare installation
#
# Headers are expected to be found in either:
#
#   <OpenLensFlare_ROOT_DIR>/OpenLensFlare.h OR
#   <OpenLensFlare_ROOT_DIR>/include/OpenLensFlare.h
#
#=============================================================================

# default header search paths
set(_OpenLensFlare_INCLUDE_SEARCH_DIRS
	"${OpenLensFlare_ROOT_DIR}"
	"${OpenLensFlare_ROOT_DIR}/include"
	"/usr/include"
	"/usr/local/include"
	"${CMAKE_SOURCE_DIR}/include"
)

# look for the library header
find_path(OpenLensFlare_INCLUDE_DIR "OpenLensFlare/OpenLensFlare.h" 
	PATHS ${_OpenLensFlare_INCLUDE_SEARCH_DIRS}
)

# default library search paths
set(_OpenLensFlare_LIBRARY_SEARCH_DIRS
	"${OpenLensFlare_ROOT_DIR}"
	"${OpenLensFlare_ROOT_DIR}/lib"
	"/usr/lib"
	"/usr/lib/include"
	"${CMAKE_SOURCE_DIR}/lib"
)

# look for the library file
find_library(OpenLensFlare_LIBRARY 
	NAMES "OpenLensFlare" 
	PATH_SUFFIXES lib lib64
	PATHS ${_OpenLensFlare_LIBRARY_SEARCH_DIRS})

# make sure everyone is okay
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenLensFlare 
	REQUIRED_VARS OpenLensFlare_INCLUDE_DIR OpenLensFlare_LIBRARY
)

# set the output variables if everything is okay
if(OpenLensFlare_FOUND)
	set(OpenLensFlare_INCLUDE_DIRS "${OpenLensFlare_INCLUDE_DIR}")
	set(OpenLensFlare_LIBRARIES "${OpenLensFlare_LIBRARY}")
	message(STATUS "OpenLensFlare_INCLUDE_DIR = ${OpenLensFlare_INCLUDE_DIR}")
	message(STATUS "OpenLensFlare_LIBRARY = ${OpenLensFlare_LIBRARY}")
endif(OpenLensFlare_FOUND)
