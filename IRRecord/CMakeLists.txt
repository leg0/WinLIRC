cmake_minimum_required(VERSION 2.8)

add_executable(IRRecord
	../winlirc/config.cpp
	../winlirc/config.h
	../winlirc/wl_string.cpp
	../winlirc/wl_string.h
	dump_config.cpp
	dump_config.h
	emulation.cpp
	emulation.h
	irdriver.cpp
	irdriver.h
	irrecord.cpp
	)

set_target_properties(IRRecord PROPERTIES  FOLDER "Tools")

target_link_libraries(IRRecord PRIVATE winlirc)