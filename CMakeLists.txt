cmake_minimum_required(VERSION 3.0)

project(proj)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED True)
set (CMAKE_BUILD_TYPE Debug)

add_library(
    capture
    source/capture.h
    source/capture.cpp
    )

add_library(
    sip
    source/sip.h
    source/sip.cpp
    ) 

add_executable(cap source/main.cpp)

target_link_libraries(cap PRIVATE capture PRIVATE sip PRIVATE -ltins)
