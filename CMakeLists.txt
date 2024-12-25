cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_COMPILER clang)
project(room)

add_executable(server server.c)
add_executable(client client.c)
