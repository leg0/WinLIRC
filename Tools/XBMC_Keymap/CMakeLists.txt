cmake_minimum_required (VERSION 2.8)

project(XBMC_Keymap)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_executable(XBMC_Keymap WIN32
	Keymap.cpp
	Keymap.h
	Keymap.rc
	KeymapDlg.cpp
	KeymapDlg.h
	ReadMe.txt
	resource.h
	stdafx.cpp
	stdafx.h
	targetver.h
	WinLIRC_48.png
	xbmcclient.h
	res/Keymap.ico
	res/Keymap.rc2
	tinyxml2/tinyxml2.cpp
	tinyxml2/tinyxml2.h
	)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

set_target_properties(XBMC_Keymap PROPERTIES  FOLDER "Tools")
