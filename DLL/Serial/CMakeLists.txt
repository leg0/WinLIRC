cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_library(Serial MODULE
	App.cpp
	Globals.cpp
	hardware.cpp
	irdriver.cpp
	Serial.cpp
	SerialDialog.cpp
	Settings.cpp
	Transmit.cpp

	winlirc.rc

    App.h
    Globals.h
    irdriver.h
    resource.h
    SerialDialog.h
    Settings.h
    stdafx.h
    Transmit.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

target_link_libraries(Serial
	winlirc
	)

configure_plugin(Serial)
