cmake_minimum_required(VERSION 3.20)

add_library(Xbox360 MODULE
	SendReceive.cpp
	Xbox360.cpp

	SendReceive.h
	)

target_link_libraries(Xbox360 PRIVATE XInput.lib)

configure_plugin(Xbox360)
