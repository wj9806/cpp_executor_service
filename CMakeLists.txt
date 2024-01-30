cmake_minimum_required(VERSION 3.22)
project(cpp_executor_service)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(cpp_executor_service
        task_queue.cpp
        task_queue.h
        thread_pool.cpp
        thread_pool.h
        main.cpp)
