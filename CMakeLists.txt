cmake_minimum_required(VERSION 2.8.8)
project(yijian)

add_subdirectory(yijian)

set(CMAKE_BUILD_TYPE "Debug")

set(CXX_FLAGS "-std=c++11 -pthread")
message(STATUS ${CXX_FLAGS})

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "${CXX_FLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CXX_FLAGS} -O3 -Wall")

include_directories(yijian)

add_executable(a.out main.cxx)

#target_link_libraries(a.out yijian)

