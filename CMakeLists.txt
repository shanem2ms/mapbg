cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release") 
project(map C CXX)

set(PROJECT_NAME map)

add_subdirectory(core)
add_subdirectory(bgfx.cmake)


cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
include(ExternalProject)

################################################################################
# Target
################################################################################
set(Main_Files
"map.cpp"
"map.h"
"map.rc"
)

add_executable(${PROJECT_NAME} WIN32 ${Main_Files})

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
    find_library(APPKITLIB AppKit)
    set (PLATFORMLIBS 
    "${FOUNDATIONLIB};"
    "${COREFOUNDATIONLIB};"
    "${METALLIB};"
    "${QUARTZCORELIB};"
    "${APPKITLIB};"
    )
endif()

message("PLT ${PLATFORMLIBS}")
target_link_libraries(${PROJECT_NAME}
    ${PLATFORMLIBS}
    ${BGFX_LIB_DIR}/bgfx.lib
    ${BGFX_LIB_DIR}/bgfx.lib
    ${BGFX_LIB_DIR}/bx.lib
    ${BGFX_LIB_DIR}/astc.lib
    ${BGFX_LIB_DIR}/bimg.lib
    ${BGFX_LIB_DIR}/astc-codec.lib    
       )
       