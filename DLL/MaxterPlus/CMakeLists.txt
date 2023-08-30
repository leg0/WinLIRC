cmake_minimum_required(VERSION 3.0)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(MaxterPlus MODULE
	Globals.cpp
	pnp.c
	MaxterPlusPlugin.cpp
	Settings.cpp

    MaxterPlus.rc

    Globals.h
    hid.h
    hidpi.h
    hidsdi.h
    hidusage.h
    resource.h
    MaxterPlusPlugin.h
    Settings.h
	)

target_link_libraries(MaxterPlus PRIVATE
    setupapi.lib
    hid.lib
	)

configure_plugin(MaxterPlus)
