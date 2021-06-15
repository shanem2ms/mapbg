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
  GIT_REPOSITORY "https://github.com/bkaradzic/bgfx.cmake.git"
  GIT_TAG "master"
  
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/bx"
)

add_dependencies(${PROJECT_NAME} Bgfx)


################################################################################
# Include directories
################################################################################

target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
    "${CMAKE_BINARY_DIR}/."
    )


target_link_libraries(${PROJECT_NAME}
    "d3dcompiler;"
    "dxgi;"
    "dxguid;"
    "d3d11;"
    "Shcore;"        
       )
