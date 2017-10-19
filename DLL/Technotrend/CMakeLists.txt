cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(Technotrend SHARED
	Globals.cpp
	hardware.cpp
	SendReceive.cpp
	Settings.cpp
	Technotrend.cpp

	Technotrend.rc

    Globals.h
    resource.h
    SendReceive.h
    Settings.h
    ttusbirapiexp.h
	)

#add_definitions(
#	-DUNICODE
#	-D_UNICODE
#	)

target_link_libraries(Technotrend
	winlirc
	${CMAKE_CURRENT_SOURCE_DIR}/TTUSBIR.lib
	)

configure_plugin(Technotrend)
