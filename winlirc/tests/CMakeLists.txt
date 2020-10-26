find_package(GTest CONFIG REQUIRED)

add_executable(winlirc_tests
	test.cpp
	config_test.cpp
	../config.cpp
	../wl_string.cpp)

target_link_libraries(winlirc_tests
	PRIVATE
		winlirc
		#GTest::gmock
		GTest::gtest
		#GTest::gmock_main
		GTest::gtest_main)

add_test(
	NAME winlirc_tests 
	COMMAND winlirc_tests)

# Copy winlirc.exe next to winlirc_tests.exe
add_custom_command(TARGET winlirc_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:winlirc>" "$<TARGET_FILE_DIR:winlirc_tests>")