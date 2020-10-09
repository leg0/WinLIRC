cmake_minimum_required(VERSION 2.8)

add_library(Hauppauge MODULE
	irremote.cpp
	stdafx.cpp
	winlirc-hauppauge-ir.cpp

    irremote.h
    stdafx.h
    targetver.h
	)

add_definitions(
    -DUNICODE
    -D_UNICODE
    )

target_link_libraries(Hauppauge
	winlirc
	)

configure_plugin(Hauppauge)
