cmake_minimum_required(VERSION 3.23)
project(lab_2 C)
set(CMAKE_C_STANDARD 99)

include_directories(include)

add_executable(lab_2 src/main.c src/strlib.c)

add_executable(child_1 src/child_1.c src/strlib.c)


