cmake_minimum_required(VERSION 3.16)
project(rec_desc)

set(CMAKE_C_STANDARD 11)

set(SOURCE_LIB rec_desc.c)

include_directories(../RPN)

add_library(rec_desc STATIC ${SOURCE_LIB})

target_link_libraries(rec_desc funcs)