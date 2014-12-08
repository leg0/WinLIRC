cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(TeVii SHARED
	Globals.cpp
	hardware.cpp
	Receive.cpp
	Settings.cpp
	TeViiRC.cpp

	TeVii.rc

    Globals.h
    Receive.h
    resource.h
    Settings.h
    TeVii.h
	)

#add_definitions(
#	-DUNICODE
#	-D_UNICODE
#	)

target_link_libraries(TeVii
	Common
	${CMAKE_CURRENT_SOURCE_DIR}/TeVii.lib
	)

configure_plugin(TeVii)
