cmake_minimum_required (VERSION 2.8)

project(WinLIRC)

find_package(Microsoft.GSL CONFIG REQUIRED)
find_package(unofficial-mongoose CONFIG REQUIRED)
find_package(taocpp-json CONFIG REQUIRED)

# taocpp-json wants C++17, but we want more recent. Don't let it downgrade this project.
set_target_properties(taocpp::json PROPERTIES INTERFACE_COMPILE_FEATURES "")


# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

set (PUBLIC_API_FILES
	include/winlirc/winlirc_api.h
	include/winlirc/WLPluginApi.h)

set (UI_FILES
	ui/bootstrap.bundle-4.5.3.min.js
	ui/bootstrap.min.css
	ui/index.html
	ui/jquery-3.5.1.min.js
	ui/knockout-3.5.1.min.js
	ui/winlirc.css
	ui/winlirc.js)

add_executable(winlirc WIN32
	config.cpp
	config.h
	constants.h
	dll.h
	drvdlg.cpp
	drvdlg.h
	globals.cpp
	globals.h
	guicon.cpp
	guicon.h
	InputPlugin.cpp
	InputPlugin.h
	irconfig.cpp
	irconfig.h
	irdriver.cpp
	irdriver.h
	ir_remote.cpp
	ir_remote.h
	IRRemote.cpp
	Plugin.cpp
	Plugin.h
	Receive.cpp
	remote.h
	resource.h
	Send.cpp
	server.cpp
	server.h
	stdafx.h
	trayicon.cpp
	trayicon.h
	version.h
	web_server.cpp
	web_server.h
	winlirc.cpp
	winlirc.h
	winlirc.rc
	wl_string.cpp
	wl_string.h
	${PUBLIC_API_FILES}
	${UI_FILES}
	)

source_group(Common FILES ${COMMON_SOURCES})
source_group("Public API" FILES ${PUBLIC_API_FILES})
source_group("UI" FILES ${UI_FILES})

set_target_properties(winlirc PROPERTIES
	ENABLE_EXPORTS on
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
	winlirc-common
	taocpp::json
	unofficial::mongoose::mongoose)

target_include_directories(winlirc
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include)


install(TARGETS winlirc 
	DESTINATION .
	ARCHIVE DESTINATION plugin_sdk/lib
	PUBLIC_HEADER DESTINATION plugin_sdk/include/winlirc
	RESOURCE DESTINATION configs)

install(FILES ${UI_FILES}
	DESTINATION ./ui)

add_subdirectory(tests)
