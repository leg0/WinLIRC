cmake_minimum_required(VERSION 2.8)

if (WL_PLATFORM MATCHES "[Xx]64")
	message("TeVii is only available in 32 bit")
else()
	add_library(TeVii MODULE
		Globals.cpp
		hardware.cpp
		Receive.cpp
		Settings.cpp
		TeViiRC.cpp

		TeVii.rc

		Globals.h
		Receive.h
		resource.h
		Settings.h
		TeVii.h
		)

	target_link_libraries(TeVii
		winlirc
		${CMAKE_CURRENT_SOURCE_DIR}/TeVii.lib
		)

	configure_plugin(TeVii)
endif()
