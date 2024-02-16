cmake_minimum_required(VERSION 3.16)
project(simple_shell C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_EXE shell.c)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.c)

add_executable(simple_shell ${SOURCE_EXE} ${SOURCES})
