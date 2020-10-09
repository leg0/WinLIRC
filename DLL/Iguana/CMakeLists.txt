cmake_minimum_required(VERSION 2.8)

if (WL_PLATFORM MATCHES "[Xx]64")
	message("Iguana is only available in 32 bit")
else()
	add_library(Iguana MODULE
		Globals.cpp
		Hardware.cpp
		Iguana.cpp
		SendReceiveData.cpp
		Settings.cpp

		Iguana.rc

		Globals.h
		iguanaIR.h
		resource.h
		SendReceiveData.h
		Settings.h
		)

	#add_definitions(
	#    -DUNICODE
	#    -D_UNICODE
	#    )

	target_link_libraries(Iguana PRIVATE
		winlirc
		${CMAKE_CURRENT_SOURCE_DIR}/iguanaIR.lib
		wsock32.lib
		)

	configure_plugin(Iguana)
endif()
