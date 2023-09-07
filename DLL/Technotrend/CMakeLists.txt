cmake_minimum_required(VERSION 3.0)

if (WL_PLATFORM MATCHES "[Xx]64")
	message("Technotrend is only available in 32 bit")
else()
	add_library(Technotrend MODULE
		Globals.cpp
		hardware.cpp
		Settings.cpp
		TechnotrendPlugin.cpp
		TechnotrendPlugin.h

		Technotrend.rc

		Globals.h
		resource.h
		Settings.h
		ttusbirapiexp.h
		)

	target_link_libraries(Technotrend PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/TTUSBIR.lib
		)

	configure_plugin(Technotrend)
endif()
