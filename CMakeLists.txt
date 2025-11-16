cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)

project(GuelderResourcesManager)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(GuelderResourcesManager STATIC
	"include/GuelderResourcesManager.hpp"
	"src/GuelderResourcesManager.cpp"
)