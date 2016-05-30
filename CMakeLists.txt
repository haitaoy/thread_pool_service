cmake_minimum_required(VERSION 3.3)
project(thread_pool_service)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

aux_source_directory(. SOURCE_FILES)

add_executable(thread_pool_service ${SOURCE_FILES})