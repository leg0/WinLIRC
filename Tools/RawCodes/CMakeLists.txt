cmake_minimum_required (VERSION 2.8)

project(RawCodes)

# Don't use MFC
#set(CMAKE_MFC_FLAG 0)

add_executable(RawCodes WIN32
	hardware.h
	irdriver.cpp
	irdriver.h
	main.cpp
	Settings.cpp
	Settings.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

configure_tool(RawCodes "Tools")
set_target_properties(RawCodes PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
