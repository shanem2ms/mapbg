
set (BGFX_INCLUDE_ROOT ${CMAKE_INSTALL_PREFIX}/include)
set (BGFX_INCLUDE ${BGFX_INCLUDE_ROOT}/bgfx)
set (BGFX_LIB ${CMAKE_INSTALL_PREFIX}/lib)

if (MSVC)
set(BX_INC_DIR_COMPAT ${BGFX_INCLUDE_ROOT}/compat/msvc)
elseif (APPLE)
set(BX_INC_DIR_COMPAT ${BGFX_INCLUDE_ROOT}/compat/osx)
endif()

################################################################################
# Source groups
################################################################################
set(Header_Files
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

if (MSVC)
set (SHADERC ${CMAKE_INSTALL_PREFIX}/bin/shaderc.exe)
set (SHADERC_PLATFORM windows)
set (SHADERC_VPROF vs_4_0)
set (SHADERC_FPROF ps_4_0)
set (SHADERC_CPROF cs_5_0)
elseif (APPLE)
find_file(SHADERC shaderc)
set (SHADERC_PLATFORM osx)
set (SHADERC_VPROF metal)
set (SHADERC_FPROF metal)
set (SHADERC_CPROF metal)
endif()

set (BGFX_SRC_DIR ${CMAKE_SOURCE_DIR}/bgfx.cmake/bgfx/src)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/vs_cubes.bin
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/vs_cubes.sc
                     -o ${CMAKE_BINARY_DIR}/vs_cubes.bin --type v --platform ${SHADERC_PLATFORM} --profile ${SHADERC_VPROF} -i ${BGFX_INCLUDE}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vs_cubes.sc ${CMAKE_CURRENT_SOURCE_DIR}/varying.def.sc
  VERBATIM)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/fs_cubes.bin
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc
                     -o ${CMAKE_BINARY_DIR}/fs_cubes.bin --type f --platform ${SHADERC_PLATFORM} --profile ${SHADERC_FPROF} -i ${BGFX_INCLUDE}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc ${CMAKE_CURRENT_SOURCE_DIR}/varying.def.sc
  VERBATIM)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/cs_erosion.bin
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/cs_erosion.sc
                     -o ${CMAKE_BINARY_DIR}/cs_erosion.bin --type c --platform ${SHADERC_PLATFORM} --profile ${SHADERC_CPROF} -i ${BGFX_INCLUDE}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cs_erosion.sc ${CMAKE_CURRENT_SOURCE_DIR}/uniforms.sh
  VERBATIM)

add_library(core STATIC ${ALL_FILES})


set(S1 ${CMAKE_BINARY_DIR}/vs_cubes.bin)
set(S2 ${CMAKE_BINARY_DIR}/fs_cubes.bin)
set(S3 ${CMAKE_BINARY_DIR}/cs_erosion.bin)

target_sources(core PUBLIC ${S1} ${S2} ${S3})

set_source_files_properties(${S1} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${S2} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set_source_files_properties(${S3} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

################################################################################
# Include directories
################################################################################

message("BGFX_INC_DIR = ${BGFX_INC_DIR}")

target_include_directories(core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
    ${BGFX_INCLUDE_ROOT}
    ${BX_INC_DIR_COMPAT}
    )
