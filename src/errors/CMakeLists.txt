cmake_minimum_required(VERSION 3.16)
project(errors)

set(CMAKE_C_STANDARD 11)

set(SOURCE_LIB errors.c)

add_library(errors STATIC ${SOURCE_LIB})