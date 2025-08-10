cmake_minimum_required (VERSION 3.22)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

project("ccensus")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

add_subdirectory(external EXCLUDE_FROM_ALL)
add_subdirectory(lib)
add_subdirectory(main)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH "Where to place compiled static libraries.")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib CACHE PATH "Where to place compiled shared libraries.")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "Where to place compiled executables.")
