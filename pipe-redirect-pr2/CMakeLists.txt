cmake_minimum_required(VERSION 3.23)
project(pipe_redirect_pr2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(pipe_redirect_pr2 main.cpp)
add_executable(threads threads.cpp)
