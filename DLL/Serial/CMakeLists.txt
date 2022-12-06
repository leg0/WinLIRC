cmake_minimum_required(VERSION 3.0)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_library(Serial MODULE
	Globals.cpp
	irdriver.cpp
	Serial.cpp
	SerialDialog.cpp
	Settings.cpp
	Transmit.cpp

	winlirc.rc

    Globals.h
    irdriver.h
    resource.h
    SerialDialog.h
    Settings.h
    stdafx.h
    Transmit.h
	)

target_compile_definitions(Serial
	PRIVATE
		UNICODE
		_UNICODE
		_AFXDLL
	)

configure_plugin(Serial)
