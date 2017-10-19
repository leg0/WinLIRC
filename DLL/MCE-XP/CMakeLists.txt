cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(MCE-XP SHARED
	Globals.cpp
	hardware.cpp
	IrDeviceList.cpp
	MCE.cpp
	Registry.cpp
	SendReceiveData.cpp
	Settings.cpp

	MCE.rc

    Globals.h
    IrDeviceList.h
    MCEDefines.h
    Registry.h
    resource.h
    SendReceiveData.h
    Settings.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

target_link_libraries(MCE-XP
	winlirc
	setupapi.lib
	hid.lib
	)

configure_plugin(MCE-XP)
