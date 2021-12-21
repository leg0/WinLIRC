cmake_minimum_required(VERSION 3.0)

add_library(Streamzap MODULE
	Globals.cpp
	hardware.cpp
	Streamzap.cpp
	StreamzapAPI.cpp

    Globals.h
    StreamzapAPI.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

target_link_libraries(Streamzap PRIVATE
	setupapi.lib
	)

configure_plugin(Streamzap)