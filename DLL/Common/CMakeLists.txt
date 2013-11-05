cmake_minimum_required (VERSION 2.8)

project (Common)

add_library(Common
	IRRemote.cpp
	Linux.cpp
	Receive.cpp
	Send.cpp

    DebugOutput.h
    Hardware.h
    IRRemote.h
    Linux.h
    LIRCDefines.h
    Receive.h
    Send.h
    WLPluginAPI.h
	)
