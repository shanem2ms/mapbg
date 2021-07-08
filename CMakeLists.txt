cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

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

add_subdirectory(bgfx.cmake)
add_subdirectory(core)

add_dependencies(core shaderc)


################################################################################
# Target
################################################################################
set(Main_Files
"map.cpp"
"map.h"
"map.rc"
)

add_executable(${PROJECT_NAME} WIN32 ${Main_Files} bgfx.cmake core)

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


target_link_libraries(${PROJECT_NAME} LINK_PUBLIC bgfx bx core
    ${PLATFORMLIBS}
       )
       
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
    "${CMAKE_BINARY_DIR}/."
    "${BGFX_INCLUDE_ROOT}"
    "${BX_INC_DIR_COMPAT}"
    "${BGFX_INC_DIR}"
    )
