cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_library(ir-tiny MODULE
    stdafx.cpp
    irdriver.cpp
    irtiny_api.cpp
    ConfigDialog.cpp
    Settings.cpp

    irtiny.rc
    irtiny.def

    irdriver.h
    resource.h
    ConfigDialog.h
    Settings.h
    stdafx.h
    )

if (MSVC)
    set_target_properties(ir-tiny PROPERTIES COMPILE_FLAGS "/Yustdafx.h /Zi")
    set_target_properties(ir-tiny  PROPERTIES LINK_FLAGS "/DEBUG")
    set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")
endif()

target_compile_definitions(ir-tiny PRIVATE UNICODE _UNICODE)

find_path(WTL_INCLUDE_DIRS "wtl/atlapp.h")
target_include_directories(ir-tiny PRIVATE
    ${WTL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/DLL
    )

configure_plugin(ir-tiny)
