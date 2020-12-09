cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(entt_playground CXX C)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Bump up warning levels appropriately for clang, gcc & msvc
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(BUILD_TESTING "Enable testing with ctest." OFF)

add_subdirectory("external/entt")
add_subdirectory("external/glm")


set(TF_BUILD_BENCHMARKS OFF
	CACHE
	STRING "Enables build of benchmarks"
	FORCE
)
set(TF_BUILD_CUDA OFF
	CACHE
	STRING "Enables build of CUDA code"
	FORCE
)
set(TF_BUILD_TESTS OFF
	CACHE
	STRING "Enables build of tests"
	FORCE
)
set(TF_BUILD_EXAMPLES OFF
	CACHE
	STRING "Enables build of examples"
	FORCE
)
add_subdirectory("external/taskflow")

####################

add_subdirectory("projects")

