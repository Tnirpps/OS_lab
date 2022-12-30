cmake_minimum_required(VERSION 3.24)
project(main C)

set(CMAKE_C_STANDARD 11)

add_executable(main ./src/main.c ./src/msgQ.c ./headers/msgQ.h)
add_executable(node ./src/client.c ./src/msgQ.c ./headers/msgQ.h)

target_link_libraries(main PRIVATE libzmq.so)
target_link_libraries(node PRIVATE libzmq.so)
