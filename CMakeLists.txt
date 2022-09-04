cmake_minimum_required(VERSION 3.4)
project(redis_ae)

set(CMAKE_CXX_STANDARD 11)

add_executable(redis_ae ae.c ae_epoll.c anet.c main.c dict.c dispatcher_servlet.c request_response.c
        controller.c controller.h constant.h)

add_executable(tcp_client tcp_demo/simple_tcp_client.c)
add_executable(tcp_server tcp_demo/simple_tcp_server.c)

add_executable(epoll_tcp_server tcp_demo/epoll_tcp_server.c)
add_executable(epoll_tcp_client tcp_demo/epoll_tcp_client.c)
