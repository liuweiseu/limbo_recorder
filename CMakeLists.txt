set(LIMBO_PROJ limbo_project)
set(LIMBO_TARGET limbo)
set(LIMBO_NAME limbo.so)
set(LIMBO_LIB m rt hashpipe hiredis stdc++)
set(LIMBO_LIB_DIR /usr/local/lib)
set(LIMBO_INCLUDE include)
set(LIMBO_SRC src)
set(LIMBO_SCRIPTS scripts/restart_recorder.sh
                  scripts/stop_recorder.sh)
set(LIMBO_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                        GROUP_READ GROUP_EXECUTE
                        WORLD_READ WORLD_EXECUTE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse4")

message("BUILD TYPE: ${CMAKE_BUILD_TYPE}")

cmake_minimum_required(VERSION 3.10)
project(${LIMBO_PROJ})
aux_source_directory(${LIMBO_SRC} SRC_LIST)
include_directories(${LIMBO_INCLUDE})
link_directories(${LIMBO_LIB_DIR})
add_library(${LIMBO_TARGET} SHARED ${SRC_LIST})
target_link_libraries(${LIMBO_TARGET} ${LIMBO_LIB})
set_target_properties(${LIMBO_TARGET} PROPERTIES PREFIX "")

install(TARGETS ${LIMBO_TARGET}
        CONFIGURATIONS Release
        LIBRARY DESTINATION lib
        PERMISSIONS ${LIMBO_PERMISSIONS})

install(FILES ${LIMBO_SCRIPTS}
        CONFIGURATIONS Release
        DESTINATION bin
        PERMISSIONS ${LIMBO_PERMISSIONS})