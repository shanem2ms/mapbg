cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-x objective-c++")

set(CMAKE_CONFIGURATION_TYPES "Debug;Release") 

set(IOSAPPNAME app2)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

set (BGFX_INCLUDE_ROOT ${CMAKE_INSTALL_PREFIX}/include)
set (BGFX_INCLUDE ${BGFX_INCLUDE_ROOT}/bgfx)
set (BGFX_LIB ${CMAKE_INSTALL_PREFIX}/lib)

################################################################################
# Source groups
################################################################################
set(Header_Files
    "entry_p.h"
    "entry.h"
    "cmd.h"
    "input.h"
)  

source_group("Header Files" FILES ${Header_Files})

set(Source_Files   
    "main.mm"
    "entry.cpp"
    "cmd.cpp"
    "input.cpp"
    "start.cpp"
    "bounds.cpp"
    "bgfx_utils.cpp"    
    )

source_group("Source Files" FILES ${Source_Files})

set(S1 ${CMAKE_BINARY_DIR}/vs_cubes.bin)
set(S2 ${CMAKE_BINARY_DIR}/fs_cubes.bin)
set(S3 ${CMAKE_BINARY_DIR}/cs_copysect.bin)
set(S4 ${CMAKE_BINARY_DIR}/cs_erosion.bin)
set(S5 ${CMAKE_BINARY_DIR}/vs_terrain.bin)

set_source_files_properties(${S1} ${S2} ${S3} ${S4} ${S5} PROPERTIES GENERATED true)
set(ALL_FILES    
    ${Header_Files} 
    ${Source_Files}
    ${S1}
    ${S2}
    ${S3}
    ${S4}
    ${S5}
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

set_target_properties(${IOSAPPNAME} PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER ${BUNDLE_ID})

set_source_files_properties(${S1} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${S2} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${S3} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${S4} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

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