cmake_minimum_required(VERSION 2.8)

if (WL_PLATFORM MATCHES "[Xx]64")
	message("Geniatech is available only in 32 bit")
else()
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
		winlirc
		${CMAKE_CURRENT_SOURCE_DIR}/LwExtLib.lib
		setupapi.lib
		)

	configure_plugin(Geniatech)
endif()
