message(STATUS "Configuring breep")

get_filename_component(BREEP_DIR ${CMAKE_SOURCE_DIR}/external/breep ABSOLUTE)

# Submodule check
directory_is_empty(is_empty "${BREEP_DIR}")
if(is_empty)
	message(FATAL_ERROR "Breep dependency is missing, maybe you didn't pull the git submodules")
endif()

set(BOOST_MIN_VERSION "1.55")
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED system)

if(NOT Boost_FOUND)
	message(FATAL_ERROR "Boost not found")
	return()
endif()

# Variables
set(BOOST_INCLUDE_DIR  ${Boost_INCLUDE_DIRS})
set(BOOST_LIBRARY  ${Boost_LIBRARIES})

get_filename_component(BREEP_INCLUDE_DIR  ${BREEP_DIR}/include  ABSOLUTE)
set(BREEP_LIBRARY "")

# Message
message("> include: ${BREEP_INCLUDE_DIR}")
message(STATUS "Configuring breep - Done")
