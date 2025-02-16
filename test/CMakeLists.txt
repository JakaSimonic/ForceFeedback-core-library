cmake_minimum_required(VERSION 3.31)
set(This FfbLibTests)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(Headers
    helpers/hidTypesExt.hpp
)
set(Sources 
    test.cpp
)
add_executable(${This} ${Sources} ${Headers})

target_link_libraries(${This} PUBLIC 
    gtest_main
    gtest
    pthread
    FfbLib
)

add_test(
    NAME ${This}
    COMMAND ${This}
)
