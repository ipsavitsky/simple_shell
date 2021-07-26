cmake_minimum_required(VERSION 3.16)
project(simple_shell C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_EXE shell.c)

include_directories(src/funcs src/errors src/rec_desc src/RPN src/stack)

add_executable(simple_shell ${SOURCE_EXE})

add_subdirectory(src/funcs)
add_subdirectory(src/errors)
add_subdirectory(src/rec_desc)
add_subdirectory(src/RPN)
add_subdirectory(src/stack)

target_link_libraries(simple_shell funcs errors rec_desc RPN stack)