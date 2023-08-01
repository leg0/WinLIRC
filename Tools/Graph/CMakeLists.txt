cmake_minimum_required (VERSION 3.0)

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
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

target_link_libraries(Graph PRIVATE winlirc opengl32.lib)

configure_tool(Graph "Tools")