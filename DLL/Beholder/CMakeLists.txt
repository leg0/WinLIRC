cmake_minimum_required(VERSION 2.8)

add_library(Beholder SHARED
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

target_link_libraries(Beholder
	winlirc
	)

configure_plugin(Beholder)
