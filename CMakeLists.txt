cmake_minimum_required(VERSION 3.31)
set(This FfbLib)

project(${This} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

set(FFB_HEADERS 
    src/HIDReportType.h
    src/FfbReportHandler.h 
    src/FfbEngine.h
    src/UserInput.h
)
set(FFB_SOURCES 
    src/FfbReportHandler.cpp
    src/FfbEngine.cpp
    src/UserInput.cpp
)

list(TRANSFORM FFB_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
list(TRANSFORM FFB_HEADERS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

add_library(${This} STATIC ${FFB_SOURCES} ${FFB_HEADERS})

add_subdirectory(test)
