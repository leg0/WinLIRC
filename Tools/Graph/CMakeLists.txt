cmake_minimum_required (VERSION 2.8)

project(Graph)

# Don't use MFC
set(CMAKE_MFC_FLAG 0)

add_executable(Graph WIN32
	guicon.cpp
	guicon.h
	irdriver.cpp
	irdriver.h
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

target_link_libraries(Graph PRIVATE winlirc opengl32.lib)

configure_tool(Graph "Tools")