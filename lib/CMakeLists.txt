add_library (ccensus_lib STATIC 
	include/utils.h
	include/project.h
	include/solution.h
	include/lib.h
	include/cmake.h
	include/backend.h
	src/lib.cpp
	src/project.cpp
	src/solution.cpp
	src/cmake.cpp
	src/data.cpp
	src/backend.cpp
)

target_include_directories(ccensus_lib PUBLIC include)

target_link_libraries(ccensus_lib PRIVATE simdjson)
