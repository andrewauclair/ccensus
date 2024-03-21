project("Lines of Code - App")

add_executable(lines_of_code main.cpp)

target_link_libraries(lines_of_code PRIVATE
	lines_of_code_lib 
)
