cmake_minimum_required(VERSION 3.0)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_library(IRToy MODULE
	Globals.cpp
	Hardware.cpp
	IRToy.cpp
	SendReceiveData.cpp
	Serial.cpp
	Settings.cpp

    IRToy.rc

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

#target_link_libraries(IRToy
#	${CMAKE_CURRENT_SOURCE_DIR}/libusb.lib
#	)

configure_plugin(IRToy)
