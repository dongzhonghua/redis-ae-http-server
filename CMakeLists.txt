cmake_minimum_required(VERSION 3.4)
project(redis_ae)

set(CMAKE_CXX_STANDARD 11)

add_executable(redis_ae ae.c ae_epoll.c anet.c main.c)

add_executable(epoll_demo epoll_demo.c)
add_executable(tcp_client tcp_client.c)
add_executable(tcp_server tcp_server.c)
