cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(Xbox360 SHARED
	SendReceive.cpp
	Xbox360.cpp

    SendReceive.h
    XInput.h
	)

#add_definitions(
#	-DUNICODE
#	-D_UNICODE
#	)

target_link_libraries(Xbox360
	Common
	${CMAKE_CURRENT_SOURCE_DIR}/XInput.lib
	)

configure_plugin(Xbox360)