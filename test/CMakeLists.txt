add_executable(ccensus_test 
	test.cpp
)

target_link_libraries(ccensus_test PRIVATE
	ccensus_lib
	Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

include(CTest)
include(Catch)
catch_discover_tests(ccensus_test)