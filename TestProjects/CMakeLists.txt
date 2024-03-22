cmake_minimum_required (VERSION 3.22)

Include(FetchContent)

cmake_policy(SET CMP0077 NEW)

project("LOC Test Projects")

FetchContent_Declare(
    libassert
    GIT_SHALLOW    TRUE
    GIT_REPOSITORY https://github.com/jeremy-rifkin/libassert
    GIT_TAG        v2.0.0
)

FetchContent_Declare(
	cpptrace
	GIT_SHALLOW		TRUE
	GIT_REPOSITORY	https://github.com/jeremy-rifkin/cpptrace
	GIT_TAG			v0.5.0
)

SET(CPPTRACE_BUILD_TESTING ON)

FetchContent_MakeAvailable(libassert cpptrace)