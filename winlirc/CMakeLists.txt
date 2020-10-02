cmake_minimum_required (VERSION 2.8)

project(WinLIRC)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_definitions(
	-DUNICODE
	-D_UNICODE
	)

set(COMMON_SOURCES
	../DLL/Common/DebugOutput.h
	../DLL/Common/enumSerialPorts.h
	../DLL/Common/fifo.h
	../DLL/Common/Hardware.h
	../DLL/Common/IRRemote.cpp
	../DLL/Common/IRRemote.h
	../DLL/Common/LIRCDefines.h
	../DLL/Common/Receive.cpp
	../DLL/Common/Receive.h
	../DLL/Common/Send.cpp
	../DLL/Common/Send.h
	../DLL/Common/UniqueHandle.h
	../DLL/Common/Win32Helpers.cpp
	../DLL/Common/Win32Helpers.h
	../DLL/Common/winlirc_api.h
	../DLL/Common/WLPluginAPI.h)

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
    dll.h
    drvdlg.h
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
	${COMMON_SOURCES}
	)

# TODO: add unit tests
# add_subdirectory(tests)

source_group(Common FILES ${COMMON_SOURCES})
set_target_properties(winlirc PROPERTIES ENABLE_EXPORTS on)

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    set (CFG ${OUTPUTCONFIG})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set_target_properties( winlirc PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${CFG} )
endforeach()

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(winlirc PRIVATE Microsoft.GSL::GSL)
