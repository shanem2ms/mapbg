cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release") 
project(map C CXX)

set(PROJECT_NAME map)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "map.h"
    "Application.h"
    "Board.h"
    "Engine.h"
    "HSLColor.h"
    "Resource.h"
    "SceneItem.h"
    "Mesh.h"
    "SimplexNoise/SimplexNoise.h"
)  

source_group("Header Files" FILES ${Header_Files})

if (MSVC)
set(Main_Files
    "map.cpp"
    "map.rc"
)
elseif(APPLE)
set(Main_Files "")
endif()    

set(Source_Files
    "Application.cpp"
    "biomegen.cpp"
    "Board.cpp"
    "Engine.cpp"
    "HSLColor.cpp"
    "SceneItem.cpp"
    "UIControl.cpp"
    "Mesh.cpp"
    "SimplexNoise/SimplexNoise.cpp"
    )

source_group("Source Files" FILES ${Source_Files} ${Main_Files})

set(ALL_FILES
    
    ${Header_Files} 
    ${Source_Files}
    ${Main_Files}
    ${CMAKE_BINARY_DIR}/vs_cubes.bin
    ${CMAKE_BINARY_DIR}/fs_cubes.bin
    ${CMAKE_BINARY_DIR}/cs_erosion.bin
)

################################################################################
# Target
################################################################################

add_executable(${PROJECT_NAME} WIN32 ${ALL_FILES})


set(BGFX_INSTALL_DIR ${CMAKE_SOURCE_DIR}/bgfx.cmake/out/install/x64-Debug)
set(BGFX_INCLUDE_ROOT ${BGFX_INSTALL_DIR}/include)
set(BGFX_LIB_DIR ${BGFX_INSTALL_DIR}/lib)

set(BX_INC_DIR ${BGFX_INCLUDE_ROOT}/bx)
set(BGFX_INC_DIR ${BGFX_INCLUDE_ROOT}/bgfx)
if (MSVC)
set(BX_INC_DIR_COMPAT ${BGFX_INCLUDE_ROOT}/compat/msvc)
elseif (APPLE)
set(BX_INC_DIR_COMPAT ${BGFX_INCLUDE_ROOT}/osx)
endif()
set(BX_BIN_DIR ${BINARY_DIR})

message(BGFX_INC ${BX_INC_DIR})

if (MSVC)
set (SHADERC_PLATFORM windows)
set (SHADERC_VPROF vs_4_0)
set (SHADERC_FPROF ps_4_0)
set (SHADERC_CPROF cs_5_0)
elseif (APPLE)
set (SHADERC_PLATFORM osx)
set (SHADERC_VPROF metal)
set (SHADERC_FPROF metal)
set (SHADERC_CPROF metal)
endif()

set (SHADERC ${CMAKE_SOURCE_DIR}/bgfx.cmake/out/install/x64-Debug/bin/shaderc${CMAKE_EXECUTABLE_SUFFIX})

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/vs_cubes.bin
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/vs_cubes.sc
                     -o ${CMAKE_BINARY_DIR}/vs_cubes.bin --type v --platform ${SHADERC_PLATFORM} --profile ${SHADERC_VPROF} -i ${BGFX_INC_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vs_cubes.sc ${CMAKE_CURRENT_SOURCE_DIR}/varying.def.sc
  VERBATIM)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/fs_cubes.bin
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc
                     -o ${CMAKE_BINARY_DIR}/fs_cubes.bin --type f --platform ${SHADERC_PLATFORM} --profile ${SHADERC_FPROF} -i ${BGFX_INC_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc ${CMAKE_CURRENT_SOURCE_DIR}/varying.def.sc
  VERBATIM)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/cs_erosion.bin
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/cs_erosion.sc
                     -o ${CMAKE_BINARY_DIR}/cs_erosion.bin --type c --platform ${SHADERC_PLATFORM} --profile ${SHADERC_CPROF} -i ${BGFX_INC_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cs_erosion.sc ${CMAKE_CURRENT_SOURCE_DIR}/uniforms.sh
  VERBATIM)

################################################################################
# Include directories
################################################################################

target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
    "${CMAKE_BINARY_DIR}/."
    "${BGFX_INCLUDE_ROOT}"
    "${BX_INC_DIR_COMPAT}"
    "${BGFX_INC_DIR}"
    )

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
       