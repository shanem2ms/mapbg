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
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "map.cpp"
    "map.rc"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    
    ${Header_Files} 
    ${Source_Files}
)

################################################################################
# Target
################################################################################

add_executable(${PROJECT_NAME} WIN32 ${ALL_FILES})


ExternalProject_Add(
  Bgfx
  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  GIT_REPOSITORY "https://github.com/bkaradzic/bgfx.cmake.git"
)

ExternalProject_Get_Property(Bgfx SOURCE_DIR)
ExternalProject_Get_Property(Bgfx BINARY_DIR)
message("Bindir ${BINARY_DIR}")


set(BX_INC_DIR ${SOURCE_DIR}/bx/include)
set(BGFX_INC_DIR ${SOURCE_DIR}/bgfx/include)
set(BX_BIN_DIR ${BINARY_DIR})
if (MSVC)
set(BX_INC_DIR_COMPAT ${SOURCE_DIR}/bx/include/compat/msvc)
endif()

add_dependencies(${PROJECT_NAME} Bgfx)


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
    "${BX_BIN_DIR}/bgfx.lib"
    "${BX_BIN_DIR}/bimg.lib"
    "${BX_BIN_DIR}/bx.lib"
    "${BX_BIN_DIR}/astc-codec.lib"
       )
