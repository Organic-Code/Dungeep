cmake_minimum_required(VERSION 3.12)
project(dungeep LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(_GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC _GLIBCXX_SANITIZE_VECTOR)

include_directories(include templates)

set(COMMON_SOURCES
        src/geometry.cpp
        src/map.cpp
        )

get_filename_component(root_dir ${CMAKE_SOURCE_DIR} REALPATH)
foreach(source ${COMMON_SOURCES})
    list(APPEND COMMON_SOURCES_ABS ${root_dir}/${source})
endforeach()

add_subdirectory(${root_dir}/tests)

add_executable(dungeep main.cpp ${COMMON_SOURCES_ABS})
