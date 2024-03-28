add_library (lines_of_code_lib STATIC 
	include/utils.h
	include/project.h
	include/solution.h
	include/lib.h
	src/lib.cpp
	src/project.cpp
	src/solution.cpp
)

target_include_directories(lines_of_code_lib PUBLIC include)
