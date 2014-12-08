cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_library(TbsQBOX SHARED
	Globals.cpp
	hardware.cpp
	Receive.cpp
	Settings.cpp
	TbsQboxRC.cpp

	TbsQbox.rc

    Globals.h
    Receive.h
    resource.h
    Settings.h
    stdafx.h
	)

#add_definitions(
#	-DUNICODE
#	-D_UNICODE
#	)

target_link_libraries(TbsQBOX
	Common
	winmm.lib
	strmiids.lib
	ksproxy.lib
	)

configure_plugin(TbsQBOX)
