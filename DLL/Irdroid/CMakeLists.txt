cmake_minimum_required(VERSION 3.0)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 0)

add_library(Irdroid MODULE
	Globals.cpp
	Hardware.cpp
	Irdroid.cpp
	SendReceiveData.cpp
	Serial.cpp
	Settings.cpp

    Irdroid.rc

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

#target_link_libraries(Irdroid PRIVATE
#	${CMAKE_CURRENT_SOURCE_DIR}/libusb.lib
#	)

configure_plugin(Irdroid)
