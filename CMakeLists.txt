cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
include(ExternalProject)


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
    "SimplexNoise/SimplexNoise.h"
)

source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "map.cpp"
    "map.rc"
    "Application.cpp"
    "biomegen.cpp"
    "Board.cpp"
    "Engine.cpp"
    "HSLColor.cpp"
    "SceneItem.cpp"
    "UIControl.cpp"
    "SimplexNoise/SimplexNoise.cpp"
    )

source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    
    ${Header_Files} 
    ${Source_Files}
    ${CMAKE_BINARY_DIR}/vs_cubes.bin
    ${CMAKE_BINARY_DIR}/fs_cubes.bin
)

################################################################################
# Target
################################################################################

add_executable(${PROJECT_NAME} WIN32 ${ALL_FILES})


ExternalProject_Add(
  Bgfx
  PREFIX "bgfx"
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  GIT_REPOSITORY "https://github.com/bkaradzic/bgfx.cmake.git"
  BUILD_BYPRODUCTS (
    ${CMAKE_BINARY_DIR}/bgfx/src/Bgfx-build/bx${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${CMAKE_BINARY_DIR}/bgfx/src/Bgfx-build/bimg${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${CMAKE_BINARY_DIR}/bgfx/src/Bgfx-build/astc-codec${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${CMAKE_BINARY_DIR}/bgfx/src/Bgfx-build/bgfx${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${CMAKE_BINARY_DIR}/bgfx/src/Bgfx-build/shaderc${CMAKE_EXECUTABLE_SUFFIX}
    )
)

ExternalProject_Get_Property(Bgfx BINARY_DIR)
ExternalProject_Get_Property(Bgfx SOURCE_DIR)
set(BGFX_SRCDIR "${SOURCE_DIR}/bgfx/src")
ExternalProject_Get_Property(Bgfx BUILD_BYPRODUCTS)
message("src ${BGFX_SRCDIR}")

add_library(BgfxLib SHARED IMPORTED)
add_dependencies(BgfxLib Bgfx)
set_target_properties(BgfxLib
    PROPERTIES IMPORTED_IMPLIB ${BINARY_DIR}/bgfx${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(BxLib SHARED IMPORTED)
add_dependencies(BxLib Bgfx)
set_target_properties(BxLib
    PROPERTIES IMPORTED_IMPLIB ${BINARY_DIR}/bx${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(BimgLib SHARED IMPORTED)
add_dependencies(BimgLib Bgfx)
set_target_properties(BimgLib
    PROPERTIES IMPORTED_IMPLIB ${BINARY_DIR}/bimg${CMAKE_STATIC_LIBRARY_SUFFIX}
)

add_library(AstcCodecLib SHARED IMPORTED)
add_dependencies(AstcCodecLib Bgfx)
set_target_properties(AstcCodecLib
    PROPERTIES IMPORTED_IMPLIB ${BINARY_DIR}/astc-codec${CMAKE_STATIC_LIBRARY_SUFFIX}
)

set(BX_INC_DIR ${SOURCE_DIR}/bx/include)
set(BGFX_INC_DIR ${SOURCE_DIR}/bgfx/include)
set(BX_BIN_DIR ${BINARY_DIR})
if (MSVC)
set(BX_INC_DIR_COMPAT ${SOURCE_DIR}/bx/include/compat/msvc)
endif()


add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/vs_cubes.bin
  COMMAND ${CMAKE_BINARY_DIR}/bgfx/src/Bgfx-build/shaderc${CMAKE_EXECUTABLE_SUFFIX} -f ${CMAKE_CURRENT_SOURCE_DIR}/vs_cubes.sc
                     -o ${CMAKE_BINARY_DIR}/vs_cubes.bin --type v -i ${BGFX_SRCDIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vs_cubes.sc
  VERBATIM)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/fs_cubes.bin
  COMMAND ${CMAKE_BINARY_DIR}/bgfx/src/Bgfx-build/shaderc${CMAKE_EXECUTABLE_SUFFIX} -f ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc
                     -o ${CMAKE_BINARY_DIR}/fs_cubes.bin --type f -i ${BGFX_SRCDIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc
  VERBATIM)
################################################################################
# Include directories
################################################################################

target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
    "${CMAKE_BINARY_DIR}/."
    "${BX_INC_DIR}"
    "${BX_INC_DIR_COMPAT}"
    "${BGFX_INC_DIR}"
    )

target_link_libraries(${PROJECT_NAME}
    "d3dcompiler;"
    "dxgi;"
    "dxguid;"
    "d3d11;"
    "Shcore;"
    BgfxLib
    BxLib
    BimgLib
    AstcCodecLib
       )
