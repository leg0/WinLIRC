cmake_minimum_required(VERSION 3.0)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_library(TbsCxt MODULE
	Globals.cpp
	hardware.cpp
	Receive.cpp
	Settings.cpp
	TbsCxtRC.cpp

	TbsCxt.rc

    Globals.h
    Receive.h
    resource.h
    Settings.h
    stdafx.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

target_link_libraries(TbsCxt PRIVATE
    winmm.lib
    strmiids.lib
    ksproxy.lib
	)

configure_plugin(TbsCxt)
