cmake_minimum_required(VERSION 3.1)

project(voip_analyzer LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED True)
set (CMAKE_BUILD_TYPE Release)


add_executable(${PROJECT_NAME}
	source/main.cpp
	source/rtp/g711.h
	source/rtp/g711.c
	source/capture.cpp
	source/capture.h
	source/console_ui.cpp
	source/console_ui.h
	source/packet_crafter.cpp
	source/packet_crafter.h
	source/sip.cpp
	source/sip.h
	source/timer.cpp
	source/timer.h
	source/rtp/codec.h
	source/rtp/codec.cpp
	source/rtp/decode.h
	source/rtp/decode.cpp
	source/rtp/rtp.h
	source/rtp/rtp.cpp
)

if(WIN32)
	#include windows version of library built from source

	set (LIBTINS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/libtins/include)
	set (WINPCAP_DEV_PACK ${CMAKE_SOURCE_DIR}/external/WpdPack/Include)

	find_library(LIBTINS tins ${CMAKE_SOURCE_DIR}/external/libtins/build/lib/Release)

	find_library(WPCAP wpcap ${CMAKE_SOURCE_DIR}/external/WpdPack/Lib/x64)
	
	find_library(PACKET Packet ${CMAKE_SOURCE_DIR}/external/WpdPack/Lib/x64)

	include_directories(${LIBTINS_INCLUDE_DIR} ${WINPCAP_DEV_PACK})

	target_link_libraries(${PROJECT_NAME} ${LIBTINS} ${WPCAP} ${PACKET} ws2_32 Iphlpapi)

	add_compile_definitions(TINS_STATIC)
else()
	#use the -ltins flag or some shit to use the locally installed lib
	include (FindPackage)

	find_package(libtins REQUIRED)

	target_link_libraries(${CMAKE_PROJECT_NAME} tins -pthread)
endif()

#target_link_libraries(cap PRIVATE console_ui PRIVATE decode PRIVATE capture PRIVATE packet_crafter  PRIVATE sip PRIVATE rtp PRIVATE codec PRIVATE g711 PRIVATE timer PRIVATE -ltins -pthread)