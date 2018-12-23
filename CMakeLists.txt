cmake_minimum_required(VERSION 3.12)
project(dungeep LANGUAGES CXX)

include(cmake/common.cmake)
include(cmake/config.cmake)
include(cmake/breep.cmake)
include(cmake/opengl.cmake)
include(cmake/SFML.cmake)
include(cmake/imgui-sfml.cmake)
include(cmake/jsoncpp.cmake)

configure_folder(
        ${CMAKE_SOURCE_DIR}/resources/
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/
        COPYONLY
)

# Variables
get_filename_component(root_dir ${CMAKE_SOURCE_DIR} REALPATH)

# Setting up Dungeep
set(COMMON_SOURCES src/map.cpp)

foreach(source ${COMMON_SOURCES})
    list(APPEND COMMON_SOURCES_ABS ${root_dir}/${source})
endforeach()

make_target(
  dungeep "dungeep" ${COMMON_SOURCES} main.cpp src/map_tester.cpp src/resource_manager.cpp
  INCLUDES
    "${root_dir}/include"
    "${root_dir}/templates"
  EXT_INCLUDES
    "${BOOST_INCLUDE_DIR}"
    "${BREEP_INCLUDE_DIR}"
    "${SFML_INCLUDE_DIR}"
    "${IMGUI_INCLUDE_DIR}"
    "${IMGUI_SFML_INCLUDE_DIR}"
    "${JSONCPP_INCLUDE_DIR}"
  OPTIONS
    cxx
    executable
    static_runtime
)

# Link dependencies
target_link_libraries(
  dungeep PRIVATE
  ${BOOST_LIBRARY}
  ${BREEP_LIBRARY}
  ${OPENGL_LIBRARY}
  ${SFML_LIBRARY}
  ${IMGUI_SFML_LIBRARY}
  ${JSONCPP_LIBRARY}
)

set_property(TARGET dungeep PROPERTY CXX_STANDARD 17)
set_property(TARGET dungeep PROPERTY CXX_STANDARD_REQUIRED ON)

# Dungeep tests
add_subdirectory(${root_dir}/tests)
