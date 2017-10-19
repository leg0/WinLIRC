cmake_minimum_required(VERSION 2.8)

add_library(Chinavision SHARED
    Chinavision.cpp
    ChinavisionAPI.cpp

    Chinavision.h
    ChinavisionAPI.h
    usb.h
    usb100.h
    usb200.h
    winusb.h
    winusbio.h
    )

target_link_libraries(Chinavision
    Common
    setupapi.lib
    winusb.lib
    )

configure_plugin(Chinavision)
