cmake_minimum_required (VERSION 3.0)

project(XBMC_Keymap)

find_package(tinyxml2 CONFIG REQUIRED)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_executable(XBMC_Keymap WIN32
	Keymap.cpp
	Keymap.h
	Keymap.rc
	KeymapDlg.cpp
	KeymapDlg.h
	resource.h
	stdafx.cpp
	stdafx.h
	targetver.h
	WinLIRC_48.png
	xbmcclient.h
	res/Keymap.ico
	res/Keymap.rc2
	)

target_link_libraries(XBMC_Keymap PRIVATE tinyxml2::tinyxml2)
target_compile_definitions(XBMC_Keymap PRIVATE UNICODE _UNICODE)

configure_tool(XBMC_Keymap "Tools")