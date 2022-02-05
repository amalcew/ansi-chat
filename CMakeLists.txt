cmake_minimum_required(VERSION 3.20)
project(ansi_irc C)

set(CMAKE_C_STANDARD 11)

add_executable(client1 client_1.c)
add_executable(client2 client_2.c)
add_executable(server server.c)
