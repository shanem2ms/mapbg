cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
include (ExternalProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release") 
project(map C CXX)

set(PROJECT_NAME map)

set(BGFX_BUILD_EXAMPLES off)
set(BGFX_CUSTOM_TARGETS off)

if (APPLE)
    set(BGFX_BUILD_TOOLS off)
endif()

ExternalProject_Add(bgfx 
    SOURCE_DIR bgfx.cmake)
ExternalProject_Add(core 
    SOURCE_DIR core)



