cmake_minimum_required(VERSION 2.8)
project(thread_pool_service)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-g -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64)

add_subdirectory(test)