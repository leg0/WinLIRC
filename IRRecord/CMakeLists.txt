cmake_minimum_required(VERSION 2.8)

add_executable(IRRecord
	config.cpp
	config.h
	dump_config.cpp
	dump_config.h
	emulation.cpp
	emulation.h
	Globals.cpp
	Globals.h
	hardware.h
	irdriver.cpp
	irdriver.h
	irrecord.cpp
	LIRCDefines.h
	)

# target_link_libraries (IRRecord
#	)

set_target_properties(IRRecord PROPERTIES  FOLDER "Tools")
