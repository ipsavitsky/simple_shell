cmake_minimum_required(VERSION 3.16)
project(funcs)

set(CMAKE_C_STANDARD 11)

set(SOURCE_LIB arithm_func.c)

include_directories(../rec_desc)

add_library(funcs STATIC ${SOURCE_LIB})

target_link_libraries(funcs rec_desc)