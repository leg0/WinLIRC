cmake_minimum_required(VERSION 3.0)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(UDP MODULE
	Globals.cpp
	hardware.cpp
	Server.cpp
	UDP.cpp

    Globals.h
    Server.h
	)

#add_definitions(
#	-DUNICODE
#	-D_UNICODE
#	)

target_link_libraries(UDP PRIVATE
	ws2_32.lib
	)

configure_plugin(UDP)
