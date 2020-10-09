cmake_minimum_required(VERSION 2.8)

add_library(Xbox360 MODULE
	SendReceive.cpp
	Xbox360.cpp

	SendReceive.h
	XInput.h
	)

if (MSVC)
	set (PLATFORM ${CMAKE_VS_PLATFORM_NAME})
else()
	set (PLATFORM ${CMAKE_GENERATOR_PLATFORM})
endif()

if (PLATFORM MATCHES "[Xx]64")
	set(XINPUT_LIB XInputX64.lib)
else()
	set(XINPUT_LIB XInput.lib)
endif()

target_link_libraries(Xbox360 PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/${XINPUT_LIB}
	)

configure_plugin(Xbox360)
