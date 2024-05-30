add_executable(ccensus main.cpp empty.cpp)

target_link_libraries(ccensus PRIVATE
	ccensus_lib simdjson CLI11
)

install(TARGETS
	ccensus
	RUNTIME
		COMPONENT Runtime
	DESTINATION bin
)