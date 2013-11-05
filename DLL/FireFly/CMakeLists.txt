cmake_minimum_required(VERSION 2.8)

add_library(FireFly SHARED
	FireFly.cpp
	Globals.cpp
	SendReceiveData.cpp
    pnp.c

    Globals.h
    hid.h
    hidpi.h
    hidsdi.h
    hidusage.h
    resource.h
    SendReceiveData.h
	)

target_link_libraries(FireFly
	Common
    setupapi.lib
    hid.lib
	)

set_target_properties(FireFly PROPERTIES FOLDER "DLL")