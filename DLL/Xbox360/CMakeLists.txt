cmake_minimum_required(VERSION 3.20)

add_library(Xbox360 MODULE
	XBox360Plugin.cpp
	XBox360Plugin.h
)

target_link_libraries(Xbox360 PRIVATE XInput.lib)

configure_plugin(Xbox360)
