cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
#set(CMAKE_MFC_FLAG 2)

add_library(Tira MODULE
	Globals.cpp
	Hardware.cpp
	Settings.cpp
	Tira.cpp
	TiraDLL.cpp

	Tira.rc

    Globals.h
    resource.h
    Settings.h
    TiraDLL.h
	)

#add_definitions(
#	-DUNICODE
#	-D_UNICODE
#	)

configure_plugin(Tira)
