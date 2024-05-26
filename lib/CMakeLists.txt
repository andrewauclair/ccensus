add_library (lines_of_code_lib STATIC 
	include/utils.h
	include/project.h
	include/solution.h
	include/lib.h
	include/cmake.h
	src/lib.cpp
	src/project.cpp
	src/solution.cpp
	src/cmake.cpp
)

target_include_directories(lines_of_code_lib PUBLIC include)

target_link_libraries(lines_of_code_lib PRIVATE simdjson)
