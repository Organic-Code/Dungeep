message(STATUS "Configuring jsoncpp")

get_filename_component(JSONCPP_DIR ${CMAKE_SOURCE_DIR}/external/jsoncpp ABSOLUTE)

# Submodule check
directory_is_empty(is_empty "${JSONCPP_DIR}")
if(is_empty)
    message(FATAL_ERROR "jsoncpp dependency is missing, maybe you didn't pull the git submodules")
endif()

add_subdirectory(${JSONCPP_DIR})

set_target_properties(jsoncpp_lib PROPERTIES FOLDER external/SFML)

target_set_output_directory(jsoncpp_lib "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
get_filename_component(JSONCPP_INCLUDE_DIR  ${JSONCPP_DIR}/include  ABSOLUTE)

set(JSONCPP_LIBRARY jsoncpp_lib)

message("> include: ${JSONCPP_INCLUDE_DIR}")