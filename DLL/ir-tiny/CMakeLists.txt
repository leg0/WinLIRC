cmake_minimum_required(VERSION 3.0)

# Use MFC in a shared DLL.
set(CMAKE_MFC_FLAG 2)

add_library(ir-tiny MODULE
    stdafx.cpp
    irdriver.cpp
    irtiny_api.cpp
    ConfigDialog.cpp

    irtiny.rc
    irtiny.def

    irdriver.h
    resource.h
    ConfigDialog.h
    stdafx.h
    )

target_compile_definitions(ir-tiny PRIVATE UNICODE _UNICODE)

find_path(WTL_INCLUDE_DIRS "atlapp.h")
target_include_directories(ir-tiny PRIVATE
    ${WTL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/DLL
    )

configure_plugin(ir-tiny)
