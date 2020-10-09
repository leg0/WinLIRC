cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(MaxterPlus MODULE
	Globals.cpp
	MaxterPlus.cpp
	pnp.c
	SendReceiveData.cpp
	Settings.cpp

    MaxterPlus.rc

    Globals.h
    hid.h
    hidpi.h
    hidsdi.h
    hidusage.h
    resource.h
    SendReceiveData.h
    Settings.h
	)

target_link_libraries(MaxterPlus PRIVATE
    setupapi.lib
    hid.lib
	)

configure_plugin(MaxterPlus)
