cmake_minimum_required(VERSION 3.31)
set(This coverage)

project(${This} CXX)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(COVERAGE_OUTPUT coverage_output)
set(COVERAGE_DIRECTORY ${CMAKE_BINARY_DIR}/${COVERAGE_OUTPUT})
file(MAKE_DIRECTORY  ${COVERAGE_DIRECTORY})

add_custom_target(${This})
set(OBJECT_DIR ${CMAKE_BINARY_DIR}/tests/coverage)
add_custom_command(TARGET ${This} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f *
    COMMAND g++ -c ${FFB_SOURCES} ${TEST_SOURCES} --coverage -I ${CMAKE_SOURCE_DIR}/src
    COMMAND g++  --coverage FfbEngine.o FfbReportHandler.o test.o UserInput.o -o ${This} -lgtest -lgtest_main
    COMMAND ${This} 
    COMMAND gcov -m -b ${FFB_SOURCES} -o .
    WORKING_DIRECTORY ${COVERAGE_DIRECTORY}
)