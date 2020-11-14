find_package(GTest CONFIG REQUIRED)

add_executable(irrecord_tests
	../dump_config.cpp
	../dump_config.h
	dump_config_test.cpp)

target_link_libraries(irrecord_tests
	PRIVATE
		winlirc
		GTest::gtest GTest::gtest_main)

add_test(
	NAME irrecord_tests 
	COMMAND irrecord_tests)

# Copy winlirc.exe next to irrecord_tests.exe
add_custom_command(TARGET irrecord_tests PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:winlirc>" "$<TARGET_FILE_DIR:irrecord_tests>")