cmake_minimum_required (VERSION 2.8)

project(Transmit)

# Don't use MFC
#set(CMAKE_MFC_FLAG 0)

add_executable(transmit WIN32
	transmit.cpp
#	transmit.rc
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

set_target_properties(transmit PROPERTIES  FOLDER "Tools")
