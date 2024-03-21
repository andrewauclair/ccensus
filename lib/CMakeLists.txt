add_library (lines_of_code_lib STATIC 
	include/project.h
	include/solution.h
	src/lib.cpp
	src/project.cpp
	src/solution.cpp
)

target_include_directories(lines_of_code_lib PUBLIC include)
