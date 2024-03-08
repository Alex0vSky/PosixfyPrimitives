cmake_minimum_required (VERSION 3.8)
project(PosixfyPrimitives CXX)
set(PROJECT_NAME PosixfyPrimitives)
SET(MY_GTEST_INCLUDE_DIRS "option_value" CACHE STRING "Missed GTEST_INCLUDE_DIRS")

################################################################################
# Source groups
################################################################################

file(GLOB A0S_SRC
     "../../src/*.h"
     "../../src/*.cpp"
)
source_group("src Files" FILES "../../PosixfyPrimitives.cpp" ${A0S_SRC} )
file(GLOB A0S_TESTS
     "../../tests/*.h"
     "../../tests/*.cpp"
)
source_group("tests Files" FILES "../../PosixfyPrimitives.cpp" ${A0S_TESTS})

set(_pch
    "../stdafx.cpp"
    "../stdafx.h"
)
source_group("_pch" FILES ${_pch})

set(ALL_FILES
    ${_pch}
    ${A0S_SRC}
    ${A0S_TESTS}
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
find_package(GTest REQUIRED)
	message(STATUS "GTEST_INCLUDE_DIRS: '${GTEST_INCLUDE_DIRS}'")
	message(STATUS "GTEST_BOTH_LIBRARIES: '${GTEST_BOTH_LIBRARIES}'")
	message(STATUS "MY_GTEST_INCLUDE_DIRS: '${MY_GTEST_INCLUDE_DIRS}'")
	message(STATUS "CMAKE_PREFIX_PATH: '${CMAKE_PREFIX_PATH}'")
	target_include_directories( ${PROJECT_NAME} PRIVATE "../../src/" )
	target_include_directories( ${PROJECT_NAME} PRIVATE ${GTEST_INCLUDE_DIRS} )
	target_include_directories( ${PROJECT_NAME} PRIVATE ${MY_GTEST_INCLUDE_DIRS} )
	target_link_libraries( ${PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} )


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
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir: '${dir}'")
endforeach()
