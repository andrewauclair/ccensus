Include(FetchContent)

Include(FindGnuplot)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
    Catch2
    GIT_SHALLOW    TRUE
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG        v3.5.3
)

FetchContent_Declare(
	ApprovalTests.cpp
	GIT_SHALLOW		TRUE
	GIT_REPOSITORY	https://github.com/approvals/ApprovalTests.cpp
	GIT_TAG			v.10.13.0
)

FetchContent_Declare(
    simdjson
    GIT_SHALLOW     TRUE
    GIT_REPOSITORY  https://github.com/simdjson/simdjson
    GIT_TAG         v3.9.2
)

FetchContent_Declare(
    CLI11
    GIT_SHALLOW     TRUE
    GIT_REPOSITORY  https://github.com/CLIUtils/CLI11
    GIT_TAG         v2.4.2
)

FetchContent_MakeAvailable(Catch2 ApprovalTests.cpp simdjson CLI11)
