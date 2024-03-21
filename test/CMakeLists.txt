project("Lines of Code - Tests")

add_executable(lines_of_code_test 
	test.cpp
)

target_link_libraries(lines_of_code_test PRIVATE
	lines_of_code_lib
	Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

include(CTest)
include(Catch)
catch_discover_tests(lines_of_code_test)