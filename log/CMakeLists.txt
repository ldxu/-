cmake_minimum_required(VERSION 3.0.0)
project(logTest)
find_package(Threads REQUIRED)
set(SOURCES
    "logTest.cpp"
    "buffer.cpp"
    "log.cpp"
)

add_executable(logTest ${SOURCES})

target_link_libraries(logTest
    ${CMAKE_THREAD_LIBS_INIT}
)