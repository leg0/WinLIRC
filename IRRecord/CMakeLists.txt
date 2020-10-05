cmake_minimum_required(VERSION 2.8)

add_executable(IRRecord
	../winlirc/config.cpp
	config.h
	dump_config.cpp
	dump_config.h
	emulation.cpp
	emulation.h
	irdriver.cpp
	irdriver.h
	irrecord.cpp
	)

# target_link_libraries (IRRecord
#	)

set_target_properties(IRRecord PROPERTIES  FOLDER "Tools")
