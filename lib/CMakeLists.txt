add_library (ccensus_lib STATIC
	src/lib.cpp
	src/project.cpp
	src/solution.cpp
	src/cmake.cpp
	src/visual_studio_frontend.cpp
	src/data.cpp
	src/backend.cpp
)

target_include_directories(ccensus_lib PUBLIC include)

target_link_libraries(ccensus_lib PRIVATE simdjson)
