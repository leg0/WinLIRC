cmake_minimum_required(VERSION 2.8)

# Use MFC in a shared DLL.
# set(CMAKE_MFC_FLAG 2)

add_library(Iguana SHARED
	Globals.cpp
	Hardware.cpp
	Iguana.cpp
	SendReceiveData.cpp
	Settings.cpp

	Iguana.rc

    Globals.h
    iguanaIR.h
    resource.h
    SendReceiveData.h
    Settings.h
	)

#add_definitions(
#    -DUNICODE
#    -D_UNICODE
#    )

target_link_libraries(Iguana
	Common
	${CMAKE_CURRENT_SOURCE_DIR}/iguanaIR.lib
	wsock32.lib
	)

configure_plugin(Iguana)
