
get_target_property(BGFXDIR bgfx SOURCE_DIR)

if (MSVC)
set(BX_INC_DIR_COMPAT ${BGFXDIR}/bx/include/compat/msvc)
elseif (APPLE)
set(BX_INC_DIR_COMPAT ${BGFX_INCLUDE_ROOT}/bx/include/compat/osx)
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

set (BGFX_SRC_DIR ${CMAKE_SOURCE_DIR}/bgfx.cmake/bgfx/src)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/vs_cubes.bin
  COMMAND shaderc -f ${CMAKE_CURRENT_SOURCE_DIR}/vs_cubes.sc
                     -o ${CMAKE_BINARY_DIR}/vs_cubes.bin --type v --platform ${SHADERC_PLATFORM} --profile ${SHADERC_VPROF} -i ${BGFX_SRC_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vs_cubes.sc ${CMAKE_CURRENT_SOURCE_DIR}/varying.def.sc
  VERBATIM)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/fs_cubes.bin
  COMMAND shaderc -f ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc
                     -o ${CMAKE_BINARY_DIR}/fs_cubes.bin --type f --platform ${SHADERC_PLATFORM} --profile ${SHADERC_FPROF} -i ${BGFX_SRC_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc ${CMAKE_CURRENT_SOURCE_DIR}/varying.def.sc
  VERBATIM)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/cs_erosion.bin
  COMMAND shaderc -f ${CMAKE_CURRENT_SOURCE_DIR}/cs_erosion.sc
                     -o ${CMAKE_BINARY_DIR}/cs_erosion.bin --type c --platform ${SHADERC_PLATFORM} --profile ${SHADERC_CPROF} -i ${BGFX_SRC_DIR}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cs_erosion.sc ${CMAKE_CURRENT_SOURCE_DIR}/uniforms.sh
  VERBATIM)

add_library(core STATIC ${ALL_FILES})

################################################################################
# Include directories
################################################################################

message("BGFX_INC_DIR = ${BGFX_INC_DIR}")

target_include_directories(core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
    "${BGFXDIR}/bgfx/include"
    "${BGFXDIR}/bx/include"
    ${BX_INC_DIR_COMPAT}
    )
