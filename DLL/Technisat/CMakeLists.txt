cmake_minimum_required(VERSION 3.0)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(Technisat MODULE
	Globals.cpp
	hardware.cpp
	Receive.cpp
	Settings.cpp
	TechnisatRC.cpp

	Technisat.rc

    b2c2_defs.h
    B2C2_Guids.h
    Globals.h
    ib2c2mpeg2avctrl.h
    ib2c2mpeg2datactrl.h
    ib2c2mpeg2tunerctrl.h
    Receive.h
    resource.h
    Settings.h
	)

#add_definitions(
#	-DUNICODE
#	-D_UNICODE
#	)

target_link_libraries(Technisat PRIVATE
	strmiids.lib
	)

configure_plugin(Technisat)
