cmake_minimum_required (VERSION 3.16)

project(winlirc-common)

find_package(Microsoft.GSL CONFIG REQUIRED)


add_library(winlirc-common STATIC
	DebugOutput.h
	enumSerialPorts.h
	fifo.h
	LIRCDefines.h
	UniqueHandle.h
	Win32Helpers.cpp
	Win32Helpers.h)

target_include_directories(winlirc-common INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR})
