cmake_minimum_required (VERSION 2.8)

project(WinLIRC)

find_package(Microsoft.GSL CONFIG REQUIRED)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

set (PUBLIC_API_FILES
	include/winlirc/PluginApi.h)

add_executable(winlirc WIN32
	config.cpp
	drvdlg.cpp
	InputPlugin.cpp
	irconfig.cpp
	irdriver.cpp
	IRRemote.cpp
	Plugin.cpp
	Receive.cpp
	Send.cpp
	server.cpp
	trayicon.cpp
	winlirc.cpp
	wl_string.cpp

    winlirc.rc

    config.h
    dll.h
    drvdlg.h
    InputPlugin.h
    irconfig.h
    irdriver.h
	Plugin.h
    remote.h
    resource.h
    server.h
    stdafx.h
    trayicon.h
    version.h
    winlirc.h
	wl_string.h
	ir_remote.cpp
	ir_remote.h
	constants.h
	wl_debug.cpp
	wl_debug.h
	${PUBLIC_API_FILES}
	)

add_library(winlirc_api INTERFACE)
target_include_directories(winlirc_api
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include)

source_group(Common FILES ${COMMON_SOURCES})
source_group("Public API" FILES ${PUBLIC_API_FILES})

set_target_properties(winlirc PROPERTIES
	PUBLIC_HEADER "${PUBLIC_API_FILES}")


foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    set (CFG ${OUTPUTCONFIG})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

    set_target_properties( winlirc PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/${CFG} )
endforeach()

target_compile_definitions(winlirc
	PRIVATE
		UNICODE
		_UNICODE
		NOMINMAX)

target_link_libraries(winlirc PRIVATE
	Microsoft.GSL::GSL
	winlirc_api
	winlirc-common)

install(TARGETS winlirc
	DESTINATION .
	ARCHIVE DESTINATION plugin_sdk/lib
	PUBLIC_HEADER DESTINATION plugin_sdk/include/winlirc
	RESOURCE DESTINATION configs
)

add_subdirectory(tests)
