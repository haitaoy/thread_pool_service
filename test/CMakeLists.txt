cmake_minimum_required(VERSION 2.8)
project(thread_pool_service)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-g -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64)

include_directories(..)

aux_source_directory(performance_test SOURCE_FILES)
add_executable(thread_pool_service ${SOURCE_FILES})

target_link_libraries(thread_pool_service -pthread)