cmake_minimum_required(VERSION 3.0)

add_executable(IRRecord
	../../winlirc/config.cpp
	../../winlirc/config.h
	../../winlirc/Plugin.cpp
	../../winlirc/Plugin.h
	../../winlirc/wl_string.cpp
	../../winlirc/wl_string.h
	dump_config.cpp
	dump_config.h
	emulation.cpp
	emulation.h
	irdriver.cpp
	irdriver.h
	irrecord.cpp
	lengths.cpp
	lengths.h
	)

configure_tool(IRRecord "Tools")

target_compile_definitions(IRRecord PRIVATE NOMINMAX)
target_link_libraries(IRRecord PRIVATE winlirc)

add_subdirectory(tests)
