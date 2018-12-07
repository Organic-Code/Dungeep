cmake_minimum_required(VERSION 3.12)
project(dungeep LANGUAGES CXX)

set(BOOST_MIN_VERSION "1.55")
set(OpenGL_GL_PREFERENCE GLVND)

get_filename_component(root_dir ${CMAKE_SOURCE_DIR} REALPATH)

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED system)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)

include(common.cmake)

#check dependencies
directory_is_empty(is_empty ${root_dir}/external/SFML)
if(is_empty)
    message(FATAL_ERROR "external/SFML is empty. Please init & pull submodules")
endif()

directory_is_empty(is_empty ${root_dir}/external/imgui)
if(is_empty)
    message(FATAL_ERROR "external/imgui is empty. Please init & pull submodules")
endif()

directory_is_empty(is_empty ${root_dir}/external/imgui-sfml)
if(is_empty)
    message(FATAL_ERROR "external/imgui-sfml is empty. Please init & pull submodules")
endif()

# configure SFML folder in IDE
foreach(sfml_target IN ITEMS sfml-system sfml-window sfml-graphics sfml-audio sfml-main)
    if(TARGET ${sfml_target})
        set_target_properties(${sfml_target} PROPERTIES FOLDER external/SFML)
    endif()
endforeach()

# Configure ImGui for ImGui-SFML
message(STATUS "ImGui configuration for SFML")
if(NOT EXISTS "${root_dir}/external/imgui/CMake_ImGui_SFML_Config_Done")
    set(tmp ${CMAKE_DISABLE_SOURCE_CHANGES})
    set(CMAKE_DISABLE_SOURCE_CHANGES OFF)
    file(READ "${root_dir}/external/imgui/imconfig-SFML.h" imconfig-SFML_content)
    file(APPEND "${root_dir}/external/imgui/imconfig.h" "${imconfig-SFML_content}")
    file(WRITE "${root_dir}/external/imgui/CMake_ImGui_SFML_Config_Done" "")
    message(STATUS "ImGui configuration for SFML - done")
    set(CMAKE_DISABLE_SOURCE_CHANGES ${tmp})
else()
    message(STATUS "ImGui configuration for SFML - already done")
endif()

# Setup ImGui-SFML
get_files(
        files
        "${root_dir}/external/imgui"
        "${root_dir}/external/imgui-sfml"
)
make_target(
        imgui-sfml "${root_dir}/external/imgui-sfml" ${files}
        INCLUDES "${root_dir}/external/SFML/include/" "${root_dir}/external/imgui" "${root_dir}/external/imgui-sfml"
        OPTIONS cxx no_warnings static_runtime
)
target_link_libraries(imgui-sfml PRIVATE "sfml-system" "sfml-window" "sfml-graphics" "sfml-audio")

# Setting up Dungeep


include_directories(include templates external/SFML/include)

set(COMMON_SOURCES src/geometry.cpp src/map.cpp)

foreach(source ${COMMON_SOURCES})
    list(APPEND COMMON_SOURCES_ABS ${root_dir}/${source})
endforeach()

add_subdirectory(${root_dir}/tests)

make_target(
        dungeep "dungeep" ${COMMON_SOURCES}
        INCLUDES include/
        EXT_INCLUDES ${root_dir}/external/SFML/include ${root_dir}/external/imgui ${root_dir}/external/imgui-sfml ${root_dir}/external/breep/include
        OPTIONS cxx executable static_runtime
)

set_property(TARGET dungeep PROPERTY CXX_STANDARD 17)
set_property(TARGET dungeep PROPERTY CXX_STANDARD_REQUIRED ON)

