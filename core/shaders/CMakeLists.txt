set (CS_SHADERS cs_erosion.sc cs_copysect.sc)
set (VS_SHADERS vs_cubes.sc vs_terrain.sc)

set (CS_BINS ${CS_SHADERS})
list(TRANSFORM CS_BINS REPLACE ".sc" ".bin" )
list(TRANSFORM CS_BINS PREPEND ${CMAKE_BINARY_DIR}/ )

set (VS_BINS ${VS_SHADERS})
list(TRANSFORM VS_BINS REPLACE ".sc" ".bin" )
list(TRANSFORM VS_BINS PREPEND ${CMAKE_BINARY_DIR}/ )


message("bin ${CS_BINS}")

set(ALL_FILES
    
    ${CMAKE_BINARY_DIR}/fs_cubes.bin
    ${CS_BINS}
    ${VS_BINS}
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


message("ct ${ALL_FILES}")
add_custom_target(shaders ALL DEPENDS ${ALL_FILES})

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/fs_cubes.bin
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc
                     -o ${CMAKE_BINARY_DIR}/fs_cubes.bin --type f --platform ${SHADERC_PLATFORM} --profile ${SHADERC_FPROF} -i ${BGFX_INCLUDE}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fs_cubes.sc ${CMAKE_CURRENT_SOURCE_DIR}/varying.def.sc
  VERBATIM)

foreach (SHDR ${CS_SHADERS})

string(REPLACE ".sc" ".bin" SHDRBIN ${SHDR})

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/${SHDRBIN}
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/${SHDR}
                     -o ${CMAKE_BINARY_DIR}/${SHDRBIN} --type c --platform ${SHADERC_PLATFORM} --profile ${SHADERC_CPROF} -i ${BGFX_INCLUDE}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHDR} ${CMAKE_CURRENT_SOURCE_DIR}/uniforms.sh
  VERBATIM)
  
endforeach()

foreach (SHDR ${VS_SHADERS})

string(REPLACE ".sc" ".bin" SHDRBIN ${SHDR})
message("VS ${SHDRBIN}")

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/${SHDRBIN}
  COMMAND ${SHADERC} -f ${CMAKE_CURRENT_SOURCE_DIR}/${SHDR}
                     -o ${CMAKE_BINARY_DIR}/${SHDRBIN} --type v --platform ${SHADERC_PLATFORM} --profile ${SHADERC_VPROF} -i ${BGFX_INCLUDE}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHDR} ${CMAKE_CURRENT_SOURCE_DIR}/uniforms.sh
  VERBATIM)
  
endforeach()


