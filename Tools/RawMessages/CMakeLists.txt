cmake_minimum_required (VERSION 2.8)

project(RawMessages)

# Don't use MFC
#set(CMAKE_MFC_FLAG 0)

add_executable(RawMessages WIN32
	Messages.cpp
	stdafx.cpp
	stdafx.h
	targetver.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

configure_tool(RawMessages "Tools")
set_target_properties(RawMessages PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
