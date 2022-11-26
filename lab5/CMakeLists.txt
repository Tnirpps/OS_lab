cmake_minimum_required(VERSION 3.23)
project(lab_5 C)
set(CMAKE_C_STANDARD 99)

add_library(
        Dyn1 SHARED
        ./headers/realisation.h
        ./src/realisation1.c
)

add_library(
        Dyn2 SHARED
        ./headers/realisation.h
        ./src/realisation2.c
)

add_executable(dyn_main ./src/main_dyn.c)

add_executable(stat_main1 ./src/main.c)
add_executable(stat_main2 ./src/main.c)

target_link_libraries(stat_main1 PRIVATE Dyn1)
target_link_libraries(stat_main2 PRIVATE Dyn2)

target_link_libraries(dyn_main PRIVATE libdl.so)
