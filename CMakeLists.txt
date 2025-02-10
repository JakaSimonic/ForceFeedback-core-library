cmake_minimum_required(VERSION 3.31)
set(This FfbLib)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

set(Headers 
    HIDReportType.h
    FfbReportHandler.h 
    FfbEngine.h
    UserInput.h
)
set(Sources 
    FfbReportHandler.cpp
    FfbEngine.cpp
    UserInput.cpp
)
add_library(${This} STATIC ${Sources} ${Headers})

add_subdirectory(test)
