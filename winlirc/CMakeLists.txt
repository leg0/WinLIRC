find_package(Microsoft.GSL CONFIG REQUIRED)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

set (PUBLIC_API_FILES
	include/winlirc/winlirc_api.h
	include/winlirc/WLPluginApi.h)

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
	settings.cpp
	trayicon.cpp
	winlircapp.cpp
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
    include/version.h
    winlircapp.h
	wl_string.h
	ir_remote.cpp
	ir_remote.h
	constants.h
	wl_debug.cpp
	wl_debug.h
	${PUBLIC_API_FILES}
	)

source_group(Common FILES ${COMMON_SOURCES})
source_group("Public API" FILES ${PUBLIC_API_FILES})

configure_file(version.h.in include/version.h @ONLY)

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
		_AFXDLL
	PUBLIC
		NOMINMAX
	)

target_link_libraries(winlirc
	PRIVATE
		winlirc-common
	PUBLIC
		Microsoft.GSL::GSL
	)

if (${CMAKE_GENERATOR} STREQUAL "Ninja")
	# https://gitlab.kitware.com/cmake/cmake/-/issues/21202
	target_link_options(winlirc
		PRIVATE
			/entry:wWinMainCRTStartup
		)
endif()

target_include_directories(winlirc
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}/include
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS winlirc 
	DESTINATION .
	ARCHIVE DESTINATION plugin_sdk/lib
	PUBLIC_HEADER DESTINATION plugin_sdk/include/winlirc
	RESOURCE DESTINATION configs
)

add_subdirectory(tests)
