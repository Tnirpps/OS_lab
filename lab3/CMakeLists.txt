cmake_minimum_required(VERSION 3.23)
project(lab_3 C)

set(CMAKE_C_STANDARD 99)

add_executable(lab_3 ./src/main.c)

set(CMAKE_C_FLAGS "-pthread")
