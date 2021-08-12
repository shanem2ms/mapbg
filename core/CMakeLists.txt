
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
    "Tile.h"
    "TileSelection.h"
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
    "Tile.cpp"
    "TileSelection.cpp"
    "HSLColor.cpp"
    "SceneItem.cpp"
    "UIControl.cpp"
    "Mesh.cpp"
    "SimplexNoise/SimplexNoise.cpp"
    )

source_group("Source Files" FILES ${Source_Files} ${Main_Files})

set (CS_SHADERS cs_erosion.sc cs_copysect.sc)

set (CS_BINS ${CS_SHADERS})
list(TRANSFORM CS_BINS REPLACE ".sc" ".bin" )
list(TRANSFORM CS_BINS PREPEND ${CMAKE_BINARY_DIR}/ )

message("bin ${CS_BINS}")

set(ALL_FILES
    
    ${Header_Files} 
    ${Source_Files}
    ${Main_Files}
    ${CMAKE_BINARY_DIR}/vs_cubes.bin
    ${CMAKE_BINARY_DIR}/fs_cubes.bin
    ${CS_BINS}
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

message(vsc ${CMAKE_BINARY_DIR}/vs_cubes.bin)
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

foreach (SHDR in ${CS_SHADERS})

string(REPLACE ".sc" ".bin" SHDRBIN ${SHDR})

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/${SHDRBIN}
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/${SHDR}
                     -o ${CMAKE_BINARY_DIR}/${SHDRBIN} --type c --platform ${SHADERC_PLATFORM} --profile ${SHADERC_CPROF} -i ${BGFX_INCLUDE}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHDR} ${CMAKE_CURRENT_SOURCE_DIR}/uniforms.sh
  VERBATIM)
  
endforeach()
add_library(core STATIC ${ALL_FILES})

################################################################################
# Include directories
################################################################################

message("BGFX_INC_DIR = ${BGFX_INC_DIR}")

target_include_directories(core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
    ${BGFX_INCLUDE_ROOT}
    ${BX_INC_DIR_COMPAT}
    )
