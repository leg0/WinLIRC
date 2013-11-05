cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(Technisat SHARED
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
    LIRCDefines.h
    Receive.h
    resource.h
    Settings.h
	)

#add_definitions(
#	-DUNICODE
#	-D_UNICODE
#	)

target_link_libraries(Technisat
	Common
	strmiids.lib
	)

set_target_properties(Technisat PROPERTIES FOLDER "DLL")
