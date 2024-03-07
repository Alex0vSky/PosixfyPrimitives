﻿cmake_minimum_required (VERSION 3.8)
project(PosixfyPrimitives CXX)
set(PROJECT_NAME PosixfyPrimitives)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "../../src/Log.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "../../PosixfyPrimitives.cpp"
    "../../src/testEvent.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(_pch
    "../stdafx.cpp"
    "../stdafx.h"
)
source_group("_pch" FILES ${_pch})

set(ALL_FILES
    ${Header_Files}
    ${Source_Files}
    ${_pch}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/../stdafx.h>"
)

################################################################################
# Output directory
################################################################################
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/../../bin/x64/${CONFIG}/")

################################################################################
# Include directories
################################################################################
find_package(GTest)
if(GTEST_FOUND)
	target_include_directories( ${PROJECT_NAME} PRIVATE ${GTEST_INCLUDE_DIRS} )
	target_link_libraries( ${PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} )
endif()

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
	"$<$<CONFIG:Debug>:"
		"_DEBUG"
	">"
	"$<$<CONFIG:Release>:"
		"NDEBUG"
	">"
)

# TODO: Add tests and install targets if needed.