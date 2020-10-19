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