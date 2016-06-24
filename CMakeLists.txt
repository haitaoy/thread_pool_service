cmake_minimum_required(VERSION 2.8)
project(thread_pool_service)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.)

aux_source_directory(. SOURCE_FILES)

#add_executable(thread_pool_service ${SOURCE_FILES} test/blocking_queue_test.cpp)
#add_executable(thread_pool_service ${SOURCE_FILES} test/task_wrapper_test.cpp)
#add_executable(thread_pool_service ${SOURCE_FILES} test/function_test/thread_pool_service_test.cpp)
add_executable(thread_pool_service ${SOURCE_FILES} test/performance_test/performance.cpp test/performance_test/worker.cpp)

target_link_libraries(thread_pool_service -pthread)