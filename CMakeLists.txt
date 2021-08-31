cmake_minimum_required(VERSION 3.16)
project(Knet C)

set(CMAKE_C_STANDARD 11)

find_package( Threads )

add_executable(Knet main.c p2p/source/server.c p2p/source/client.c p2p/source/node.c p2p/source/threadpool.c)

target_link_libraries( Knet ${CMAKE_THREAD_LIBS_INIT} )