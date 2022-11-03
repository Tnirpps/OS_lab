cmake_minimum_required(VERSION 3.23)
project(lab_4 C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_FLAGS "-lrt")
set(CMAKE_C_FLAGS "-pthread")

add_executable(lab_4 ./src/main.c)
add_executable(child ./src/child.c)


target_link_libraries(lab_4 PRIVATE librt.so)
target_link_libraries(child PRIVATE librt.so)
