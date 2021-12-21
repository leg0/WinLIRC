cmake_minimum_required(VERSION 3.0)

add_library(Beholder MODULE
	Beholder.cpp
	BeholdRC.cpp
	Globals.cpp
	hardware.cpp
	SendReceiveData.cpp

    BeholdRC.h
    Globals.h
    resource.h
    SendReceiveData.h
	)

configure_plugin(Beholder)
