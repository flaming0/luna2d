project(luac)
cmake_minimum_required(VERSION 2.8)

set(LUA_PATH ${PROJECT_SOURCE_DIR}/../../thirdparty/lua)

include_directories(${LUA_PATH})
aux_source_directory(${LUA_PATH} SOURCES)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

add_executable(luac ${SOURCES})
