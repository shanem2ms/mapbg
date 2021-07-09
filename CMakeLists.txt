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

set (BGFX_INCLUDE_ROOT ${CMAKE_INSTALL_PREFIX}/include)
set (BGFX_INCLUDE ${BGFX_INCLUDE_ROOT}/bgfx)
set (BGFX_LIB ${CMAKE_INSTALL_PREFIX}/lib)

ExternalProject_Add(
  Bgfx
  PREFIX "bgfx"
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  GIT_REPOSITORY "https://github.com/bkaradzic/bgfx.cmake.git"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
   BUILD_BYPRODUCTS (
    ${BGFX_LIB}/bx${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${BGFX_LIB}/bimg${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${BGFX_LIB}/astc-codec${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${BGFX_LIB}/bgfx${CMAKE_STATIC_LIBRARY_SUFFIX}
    )
)

add_subdirectory(core)
add_dependencies(core Bgfx)


################################################################################
# Target
################################################################################
set(Main_Files
"map.cpp"
"map.h"
"map.rc"
)

add_executable(${PROJECT_NAME} WIN32 ${Main_Files} Bgfx core)

if (MSVC)
    set (PLATFORMLIBS
    "d3dcompiler;"
    "dxgi;"
    "dxguid;"
    "d3d11;"
    "Shcore;"
)
elseif (APPLE)
find_library(FOUNDATIONLIB Foundation)
find_library(COREFOUNDATIONLIB CoreFoundation)
find_library(METALLIB Metal)
find_library(QUARTZCORELIB QuartzCore)
find_library(METALKITLIB MetalKit)
find_library(UIKITLIB UIKit)
find_library(MODELIOLIB ModelIO)
set (PLATFORMLIBS 
"${FOUNDATIONLIB};"
"${COREFOUNDATIONLIB};"
"${METALLIB};"
"${QUARTZCORELIB};"
"${METALKITLIB};"
"${UIKITLIB};"
"${MODELIOLIB};"
    )
endif()

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC 
    ${BGFX_LIB}/bgfx.lib 
    ${BGFX_LIB}/astc-codec.lib 
    ${BGFX_LIB}/bimg.lib 
    ${BGFX_LIB}/bx.lib core
    ${PLATFORMLIBS}
       )
       
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
    "${CMAKE_BINARY_DIR}/."
    "${BGFX_INCLUDE_ROOT}"
    "${BX_INC_DIR_COMPAT}"
    "${BGFX_INC_DIR}"
    )