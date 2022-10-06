cmake_minimum_required(VERSION 3.23)
project(pipe_threads_cv2)

set(CMAKE_CXX_STANDARD 14)

add_executable(pipe_threads_cv2 procesy_max.cpp)
add_executable(threads_max threads_max.cpp)
add_executable(process_file_read process_file_read.cpp)
add_executable(threads_file_read threads_file_read.cpp)
add_executable(exec_test exec_test.cpp)
