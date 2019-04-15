message(STATUS "Configuring imterm")

get_filename_component(IMTERM_DIR ${CMAKE_SOURCE_DIR}/external/imterm ABSOLUTE)

# Submodule check
directory_is_empty(is_empty "${IMTERM_DIR}")
if(is_empty)
    message(FATAL_ERROR "imterm dependency is missing, maybe you didn't pull the git submodules")
endif()

add_subdirectory(${IMTERM_DIR})

set(IMTERM_INCLUDE_DIR ${IMTERM_DIR}/include)

add_compile_definitions(IMTERM_ENABLE_REGEX)

message("> include: ${IMTERM_INCLUDE_DIR}")