
set (BGFX_INCLUDE_ROOT ${CMAKE_INSTALL_PREFIX}/include)
set (BGFX_INCLUDE ${BGFX_INCLUDE_ROOT}/bgfx)
set (BGFX_LIB ${CMAKE_INSTALL_PREFIX}/lib)

if (MSVC)
set(BX_INC_DIR_COMPAT ${BGFX_INCLUDE_ROOT}/compat/msvc)
elseif (APPLE)
set(BX_INC_DIR_COMPAT ${BGFX_INCLUDE_ROOT}/compat/osx)
endif()

add_subdirectory(shaders)

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

set(ALL_FILES
    
    ${Header_Files} 
    ${Source_Files}
    ${Main_Files}
)

add_library(core STATIC ${ALL_FILES})

add_dependencies(core shaders)
################################################################################
# Include directories
################################################################################

message("BGFX_INC_DIR = ${BGFX_INC_DIR}")

target_include_directories(core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/."
    ${BGFX_INCLUDE_ROOT}
    ${BX_INC_DIR_COMPAT}
    )
