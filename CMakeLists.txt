cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project(ResourcesManager)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(ResourcesManager STATIC
	"include/ResourcesManager.hpp"
	"src/ResourcesManager.cpp"
)