cmake_minimum_required(VERSION 3.16)
project(RPN)

set(CMAKE_C_STANDARD 11)

set(SOURCE_LIB RPN.c)

include_directories(../stack)

add_library(RPN STATIC ${SOURCE_LIB})

target_link_libraries(RPN stack)