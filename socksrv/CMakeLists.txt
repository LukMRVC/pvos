cmake_minimum_required(VERSION 3.16)
project(socksrv)

set(CMAKE_CXX_STANDARD 14)

add_executable(socksrv socket_srv.cpp)
add_executable(sockcl socket_cl.cpp)
add_executable(socksrvt socket_srv_thread.cpp)

target_link_libraries(socksrv rt pthread)
target_link_libraries(socksrvt rt pthread)
target_link_libraries(sockcl rt pthread)