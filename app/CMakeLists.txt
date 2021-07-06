cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-x objective-c++")

set(CMAKE_CONFIGURATION_TYPES "Debug;Release") 
project(app C CXX)

set(PROJECT_NAME app)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
message("CM ${CMAKE_MODULE_PATH}")
################################################################################
# Source groups
################################################################################
set(Header_Files
    "AppDelegate.h"
    "Renderer.h"
    "GameViewController.h"
    "ShaderTypes.h"
)  

source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "AppDelegate.m"
    "Renderer.m"
    "GameViewController.m"
    "main.m"
    "Main.storyboard"
    "Shaders.metal"
    )

set(Metal_Shaders
    "Shaders.metal"
)

source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    
    ${Header_Files} 
    ${Source_Files}
)

################################################################################
# Target
################################################################################

add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${ALL_FILES})

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

set(MainStoryboard "Main.storyboard")
set(Assets "Assets.xcassets")
set(CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.mapbg.shane")
set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "73CP3TPHE9")

target_sources(${PROJECT_NAME} PUBLIC ${MainStoryboard} ${Assets})

set_source_files_properties(${MainStoryboard} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${Metal_Shaders} PROPERTIES LANGUAGE METAL)
set_source_files_properties(${Assets} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
target_link_libraries(${PROJECT_NAME}
    ${PLATFORMLIBS})
