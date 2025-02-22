cmake_minimum_required(VERSION 3.31)
set(This FfbLibCoverage)

project(${This} CXX)

set(Headers
    ../test/helpers/hidTypesExt.hpp
)
set(Sources 
    ../test/test.cpp
)
add_executable(${This} ${Sources} ${Headers})
target_include_directories(${This} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../src")
target_compile_options(${This} PUBLIC  -g -O0 --coverage -fprofile-arcs -ftest-coverage)
target_link_libraries(${This} PUBLIC 
    gtest_main
    gtest
    pthread
    FfbLib
    gcov
)


# add_custom_target(gcov
#     COMMAND mkdir -p coverage
#     COMMAND ${CMAKE_MAKE_PROGRAM} test
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#     )
# add_custom_command(TARGET gcov
#     COMMAND echo "=================== GCOV ===================="
#     COMMAND gcov -b ${CMAKE_SOURCE_DIR}/*.cpp -o ${OBJECT_DIR}
#     | grep -A 5 "Adder.cpp" > CoverageSummary.tmp
#     COMMAND cat CoverageSummary.tmp
#     COMMAND echo "-- Coverage files have been output to ${CMAKE_BINARY_DIR}/coverage"
#     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
#     )