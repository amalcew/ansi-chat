cmake_minimum_required(VERSION 3.20)
project(ansi_irc C)

set(CMAKE_C_STANDARD 11)

add_executable(test test.c)
add_executable(client1 source/client_1.c)
add_executable(client2 source/client_2.c)
add_executable(server source/server.c source/aux.c source/user.c)
