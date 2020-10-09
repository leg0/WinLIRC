cmake_minimum_required(VERSION 2.8)

add_library(Chinavision MODULE
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

target_link_libraries(Chinavision PRIVATE
    setupapi.lib
    winusb.lib
    )

configure_plugin(Chinavision)
