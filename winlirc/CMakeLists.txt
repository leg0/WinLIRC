cmake_minimum_required (VERSION 2.8)

project(WinLIRC)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

add_executable(winlirc WIN32
	config.cpp
	drvdlg.cpp
	globals.cpp
	guicon.cpp
	InputPlugin.cpp
	irconfig.cpp
	irdriver.cpp
	server.cpp
	trayicon.cpp
	winlirc.cpp

    winlirc.rc

    config.h
    drvdlg.h
    dumpcfg.h
    globals.h
    guicon.h
    InputPlugin.h
    irconfig.h
    irdriver.h
    remote.h
    resource.h
    server.h
    stdafx.h
    trayicon.h
    version.h
    winlirc.h
	)

# TODO: add unit tests
# add_subdirectory(tests)

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    set (CFG ${OUTPUTCONFIG})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set_target_properties( winlirc PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${CFG} )
endforeach()
