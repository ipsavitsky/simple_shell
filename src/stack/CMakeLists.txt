cmake_minimum_required(VERSION 3.16)
project(stack)

set(CMAKE_C_STANDARD 11)

set(SOURCE_LIB stack.c)

include_directories(../errors)

add_library(stack STATIC ${SOURCE_LIB})

target_link_libraries(stack errors)