cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(fireworks_lib CXX)

add_library(fireworks_lib INTERFACE
	#./fireworks_lib.hpp
)

target_include_directories(fireworks_lib INTERFACE ".")

target_link_libraries(fireworks_lib INTERFACE
	EnTT::EnTT
	glm::glm
)

##########################

