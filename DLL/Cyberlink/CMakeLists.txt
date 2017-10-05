cmake_minimum_required(VERSION 2.8)

set(CMAKE_MFC_FLAG 0)

add_library(Cyberlink SHARED
	CyberLink.cpp
	Globals.cpp
	Hardware.cpp
	ReceiveData.cpp
	Globals.h
	ReceiveData.h
	resource.h

	USB/headers/usb.h
	USB/headers/usb100.h
	USB/headers/usb200.h
	USB/headers/winusb.h
	USB/headers/winusbio.h
)

add_definitions(
	-DUNICODE
	-D_UNICODE
	-DWIN
	)

if (CMAKE_CL_64)
	set(ARCH amd64)
else()
	set(ARCH x86)
endif()

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/USB/libs/${ARCH})

target_link_libraries(Cyberlink
	Common
	setupapi
	winusb
	)

configure_plugin(Cyberlink)
