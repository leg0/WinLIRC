cmake_minimum_required (VERSION 3.0)

project(RawCodes)

# Don't use MFC
#set(CMAKE_MFC_FLAG 0)

add_executable(RawCodes WIN32
	irdriver.cpp
	irdriver.h
	main.cpp
	Settings.cpp
	Settings.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

configure_tool(RawCodes "Tools")
set_target_properties(RawCodes PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")

target_link_libraries(RawCodes PRIVATE winlirc)
