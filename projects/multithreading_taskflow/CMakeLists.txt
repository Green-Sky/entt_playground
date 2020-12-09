cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(mt_taskflow CXX)

add_executable(mt_fireworks
	./fireworks_lib.hpp
	./mt_fireworks.cpp
)

target_link_libraries(mt_fireworks
	EnTT::EnTT
	Taskflow
	glm::glm
)

##########################

