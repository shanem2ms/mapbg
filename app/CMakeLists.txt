cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-x objective-c++")

set(CMAKE_CONFIGURATION_TYPES "Debug;Release") 

set(IOSAPPNAME app)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

set (BGFX_INCLUDE_ROOT ${CMAKE_INSTALL_PREFIX}/include)
set (BGFX_INCLUDE ${BGFX_INCLUDE_ROOT}/bgfx)
set (BGFX_LIB ${CMAKE_INSTALL_PREFIX}/lib)
message("BGFX_INCLUDE ${BGFX_INCLUDE}")
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
    "Renderer.mm"
    "GameViewController.m"
    "main.mm"
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

add_executable(${IOSAPPNAME} MACOSX_BUNDLE ${ALL_FILES})

find_library(FOUNDATIONLIB Foundation)
find_library(COREFOUNDATIONLIB CoreFoundation)
find_library(METALLIB Metal)
find_library(QUARTZCORELIB QuartzCore)
find_library(METALKITLIB MetalKit)
find_library(UIKITLIB UIKit)
find_library(MODELIOLIB ModelIO)
find_library(OPENGL OpenGLES)


set (PLATFORMLIBS 
"${FOUNDATIONLIB};"
"${COREFOUNDATIONLIB};"
"${METALLIB};"
"${QUARTZCORELIB};"
"${METALKITLIB};"
"${UIKITLIB};"
"${MODELIOLIB};"
"${OPENGL};"
)

set(MainStoryboard "Main.storyboard")
set(Assets "Assets.xcassets")

target_sources(${IOSAPPNAME} PUBLIC ${MainStoryboard} ${Assets})

set_source_files_properties(${MainStoryboard} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${Metal_Shaders} PROPERTIES LANGUAGE METAL)
set_source_files_properties(${Assets} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

target_include_directories(${IOSAPPNAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/"
    "${CMAKE_SOURCE_DIR}/core/"
    "${CMAKE_BINARY_DIR}/"
    "${BGFX_INCLUDE_ROOT}"
    "${BX_INC_DIR_COMPAT}"
    "${BGFX_INC_DIR}"
    )

target_link_libraries(${IOSAPPNAME} LINK_PUBLIC
    ${PLATFORMLIBS}
    core
    ${BGFX_LIB}/${CMAKE_STATIC_LIBRARY_PREFIX}bgfx${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${BGFX_LIB}/${CMAKE_STATIC_LIBRARY_PREFIX}astc-codec${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${BGFX_LIB}/${CMAKE_STATIC_LIBRARY_PREFIX}bimg${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${BGFX_LIB}/${CMAKE_STATIC_LIBRARY_PREFIX}bx${CMAKE_STATIC_LIBRARY_SUFFIX}
    ${PLATFORMLIBS}
       )