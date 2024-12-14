cmake_minimum_required(VERSION 3.0.0)
project(Server)
# 设置编译器标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
include_directories(
    ${CMAKE_SOURCE_DIR}/include
)
set(SRC_DIR1 ${CMAKE_SOURCE_DIR}/log)
set(SRC_DIR2 ${CMAKE_SOURCE_DIR}/net)
set(SRC_DIR3 ${CMAKE_SOURCE_DIR}/signal)
set(SRC_DIR4 ${CMAKE_SOURCE_DIR}/utils)
set(SRC_DIR5 ${CMAKE_SOURCE_DIR}/proc)
set(SRC_DIR6 ${CMAKE_SOURCE_DIR}/app)

find_package(Threads REQUIRED)

aux_source_directory(${SRC_DIR1}
SRC_FILES1)
aux_source_directory(${SRC_DIR2}
SRC_FILES2)
aux_source_directory(${SRC_DIR3}
SRC_FILES3)
aux_source_directory(${SRC_DIR4}
SRC_FILES4)
aux_source_directory(${SRC_DIR5}
SRC_FILES5)
aux_source_directory(${SRC_DIR6}
SRC_FILES6)

set(SOURCES
${SRC_FILES1}
${SRC_FILES2}
${SRC_FILES3}
${SRC_FILES4}
${SRC_FILES5}
${SRC_FILES6}
)

add_executable(Server server.cpp ${SOURCES})
target_link_libraries(Server
    ${CMAKE_THREAD_LIBS_INIT}
)
