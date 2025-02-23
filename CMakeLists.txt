cmake_minimum_required(VERSION 3.31)
set(This FFbCoreLibProject)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
#add_subdirectory(coverage)
