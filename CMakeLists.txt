cmake_minimum_required(VERSION 3.0)

project(proj)

set (CMAKE_CXX_STANDARD 14)
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

add_library(
    decode
    source/rtp/decode.h
    source/rtp/decode.cpp
    )

add_library(
    codec
    source/rtp/codec.h
    source/rtp/codec.cpp
    )

add_library(
    g711
    source/rtp/g711.h
    source/rtp/g711.c
    )

add_library(
    packet_crafter
    source/packet_crafter.h
    source/packet_crafter.cpp
    )

add_library(
    rtp
    source/rtp/rtp.h
    source/rtp/rtp.cpp
    )



add_executable(cap source/main.cpp)

target_link_libraries(cap PRIVATE decode PRIVATE capture PRIVATE packet_crafter PRIVATE sip PRIVATE rtp PRIVATE codec PRIVATE g711 PRIVATE -ltins -lpcap -lopus -pthread)

