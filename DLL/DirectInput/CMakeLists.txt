cmake_minimum_required(VERSION 2.8)

# Don't use MFC
set(CMAKE_MFC_FLAG 0)

add_library(DirectInput MODULE
    directinput.cpp
    directinput.rc
    directinput.def
    )

if (MSVC)
    set_target_properties(DirectInput PROPERTIES LINK_FLAGS "/DEBUG")
endif()

add_definitions(
    -DUNICODE
    -D_UNICODE
    -DDIRECTINPUT_EXPORTS
    -DWIN32
    )

target_link_libraries(DirectInput PRIVATE winlirc dinput8.lib dxguid.lib)

configure_plugin(DirectInput)
