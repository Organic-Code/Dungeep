cmake_minimum_required(VERSION 3.12)
project(dungeep LANGUAGES CXX)

include(cmake/common.cmake)
include(cmake/config.cmake)
include(cmake/breep.cmake)
include(cmake/opengl.cmake)
include(cmake/SFML.cmake)
include(cmake/imgui-sfml.cmake)
include(cmake/jsoncpp.cmake)
include(cmake/spdlog.cmake)
include(cmake/fmt.cmake)
include(cmake/imterm.cmake)

configure_folder(
        ${CMAKE_SOURCE_DIR}/resources/
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/
        COPYONLY
)

# Variables
get_filename_component(root_dir ${CMAKE_SOURCE_DIR} REALPATH)

# Setting up Dungeep

#set(COMMON_SOURCES)
#foreach(source ${COMMON_SOURCES})
#    list(APPEND COMMON_SOURCES_ABS ${root_dir}/${source})
#endforeach()

set(DUNGEEP_SOURCES
        src/environment/map.cpp
        src/environment/world.cpp
        src/environment/world_objects/creature.cpp
        src/environment/world_objects/item.cpp
        src/environment/world_objects/mob.cpp
        src/environment/world_objects/player.cpp
        src/display/map_tester.cpp
        src/display/terminal_commands.cpp
        src/utils/resource_manager.cpp

        src/main.cpp
        )

make_target(
  dungeep "dungeep" ${DUNGEEP_SOURCES}
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
    "${SPDLOG_INCLUDE_DIR}"
    "${FMT_INCLUDE_DIR}"
    "${IMTERM_INCLUDE_DIR}"
  OPTIONS
    cxx
    executable
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
  ${SPDLOG_LIBRARY}
  ${FMT_LIBRARY}
  ${IMTERM_LIBRARY}
)

set_property(TARGET dungeep PROPERTY CXX_STANDARD 17)
set_property(TARGET dungeep PROPERTY CXX_STANDARD_REQUIRED ON)

# Dungeep tests
add_subdirectory(${root_dir}/tests)

#set(CMAKE_VERBOSE_MAKEFILE 1)