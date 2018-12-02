cmake_minimum_required(VERSION 3.12)
project(dungeep)

set(CMAKE_CXX_STANDARD 17)

include_directories(include templates)

add_executable(dungeep main.cpp src/geometry.cpp)