cmake_minimum_required(VERSION 3.23)
project(cp C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_FLAGS "-lrt")
set(CMAKE_C_FLAGS "-pthread")

add_executable(cp ./src/main.c)
add_executable(user ./src/user.c)


target_link_libraries(cp PRIVATE librt.so)
target_link_libraries(user PRIVATE librt.so)
