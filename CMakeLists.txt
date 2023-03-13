set(LIMBO_PROJ limbo_project)
set(LIMBO_TARGET limbo)
set(LIMBO_LIB m rt hashpipe hiredis stdc++)
set(LIMBO_LIB_DIR /usr/local/lib)
set(LIMBO_INCLUDE include)
set(LIMBO_SRC src)
set(LIMBO_SCRIPTS scripts/restart_recorder.sh
                  scripts/stop_recorder.sh)

message("DEST: ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse4")

cmake_minimum_required(VERSION 3.10)
project(${LIMBO_PROJ})
aux_source_directory(${LIMBO_SRC} SRC_LIST)
include_directories(${LIMBO_INCLUDE})
link_directories(${LIMBO_LIB_DIR})
add_library(${LIMBO_TARGET} SHARED ${SRC_LIST})
target_link_libraries(${LIMBO_TARGET} ${LIMBO_LIB})

install(TARGETS ${LIMBO_TARGET}
        LIBRARY DESTINATION lib PERMISSIONS WORLD_EXECUTE)

install(FILES ${LIMBO_SCRIPTS}
        DESTINATION bin PERMISSIONS WORLD_EXECUTE)