cmake_minimum_required(VERSION 2.8)

add_library(Geniatech SHARED
	GeniatechRC.cpp
	Globals.cpp
	hardware.cpp
	Receive.cpp
	Settings.cpp

    Geniatech.rc

    Globals.h
    LWExtDLL.h
    Receive.h
    resource.h
    Settings.h
	)

target_link_libraries(Geniatech
	Common
	${CMAKE_CURRENT_SOURCE_DIR}/LwExtLib.lib
    setupapi.lib
	)

set_target_properties(Geniatech PROPERTIES FOLDER "DLL")