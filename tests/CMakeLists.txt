cmake_minimum_required(VERSION 3.12)
project(dungeep_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Catch2 REQUIRED)

set(TEST_SOURCES quadtree_test.cpp geometry_test.cpp)
set(CMAKE_CXX_FLAGS -g)

add_executable(dungeep_tests main.cpp ${COMMON_SOURCES_ABS} ${TEST_SOURCES})
target_link_libraries(dungeep_tests Catch2::Catch2 m)

include(CTest)
include(Catch)
catch_discover_tests(dungeep_tests)
