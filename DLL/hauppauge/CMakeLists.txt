cmake_minimum_required(VERSION 3.0)

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

configure_plugin(Hauppauge)
