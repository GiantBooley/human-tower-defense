cmake_minimum_required(VERSION 3.10)
project(HumanTowerDefense)

set(CMAKE_CXX_STANDARD 17)

# include
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(HumanTowerDefense
	src/main.cpp
	src/glad.c
)

target_link_libraries(HumanTowerDefense
	${CMAKE_CURRENT_SOURCE_DIR}/lib/libglfw3.a
	openal
)
