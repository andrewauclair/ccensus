add_library (ccensus_lib STATIC
	src/cmake.cpp
	src/visual_studio_frontend.cpp
	src/data.cpp
	src/backend.cpp
)

target_include_directories(ccensus_lib PUBLIC include)

target_link_libraries(ccensus_lib PRIVATE simdjson nlohmann_json)
