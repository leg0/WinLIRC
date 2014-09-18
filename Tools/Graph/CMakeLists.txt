cmake_minimum_required (VERSION 2.8)

project(Graph)

# Don't use MFC
set(CMAKE_MFC_FLAG 0)

add_executable(Graph WIN32
	guicon.cpp
	guicon.h
	hardware.h
	irdriver.cpp
	irdriver.h
	LIRCDefines.h
	main.cpp
	Opengl.cpp
	Opengl.h
	Settings.cpp
	Settings.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

set_target_properties(Graph PROPERTIES  FOLDER "Tools")


target_link_libraries(Graph opengl32.lib)