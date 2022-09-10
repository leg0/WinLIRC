cmake_minimum_required (VERSION 3.16)

project(winlirc-common)

find_package(Microsoft.GSL CONFIG REQUIRED)


add_library(winlirc-common STATIC
	include/winlirc-common/DebugOutput.h
	include/winlirc-common/enumSerialPorts.h
	include/winlirc-common/Event.h
	include/winlirc-common/fifo.h
	include/winlirc-common/UniqueHandle.h
	Win32Helpers.cpp
	include/winlirc-common/Win32Helpers.h)

target_include_directories(winlirc-common
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include)
