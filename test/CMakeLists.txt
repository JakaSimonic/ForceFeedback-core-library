cmake_minimum_required(VERSION 3.31)
set(This tests)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(TEST_HEADERS
    src/helpers/hidTypesExt.hpp
)
set(TEST_SOURCES 
    src/test.cpp
)
add_executable(${This} EXCLUDE_FROM_ALL ${TEST_SOURCES} ${TEST_HEADERS})

list(TRANSFORM TEST_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
list(TRANSFORM TEST_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

# target_include_directories(${This} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(${This} PUBLIC "${CMAKE_SOURCE_DIR}/src")

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

add_subdirectory(coverage)