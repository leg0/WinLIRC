cmake_minimum_required(VERSION 2.8)

add_library(FireFly MODULE
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

target_link_libraries(FireFly PRIVATE
    setupapi.lib
    hid.lib
	)

configure_plugin(FireFly)

