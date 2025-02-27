
# GuelderResourcesManager

A very simple lib that adds so-called ResourcesManager, whose main task is to store data from some confing file (default is `"Resources/config.txt"`). Also it can excecute console commands via pipe.

BUILD:

Use CMake to build this library with your main project. You need to download this code and do the following inside your CMakeLists.txt:

```
add_subdirectory("External/GuelderResourcesManager" "${CMAKE_CURRENT_BINARY_DIR}/GuelderResourcesManager")
target_link_libraries(${PROJECT_NAME} PUBLIC GuelderResourcesManager)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/External/GuelderResourcesManager/include")
```