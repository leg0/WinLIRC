cmake_minimum_required(VERSION 3.0)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_library(IRMan MODULE
	Globals.cpp
	hardware.cpp
	IRMan.cpp
	SendReceiveData.cpp
	Serial.cpp
	Settings.cpp

    IRMan.rc

    Globals.h
    resource.h
    SendReceiveData.h
    Serial.h
    Settings.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

configure_plugin(IRMan)
