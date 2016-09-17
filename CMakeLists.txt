cmake_minimum_required(VERSION 2.8.8)
project(yijian)
add_executable(a.out main.cxx)
add_executable(server.out server.cxx)
add_executable(client.out client.cxx)

set(CMAKE_BUILD_TYPE "Debug")
set(CXX_FLAGS "-std=c++14 -pthread -lev")
message(STATUS ${CXX_FLAGS})
#set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS_DEBUG "${CXX_FLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CXX_FLAGS} -O3 -Wall")

add_subdirectory(yijian)
include_directories(yijian)
#target_link_libraries(a.out yijian)

#
set(EXT_PROJECTS_DIR externals)
# Includes Catch in the project:
add_subdirectory(${EXT_PROJECTS_DIR}/catch)
include_directories(${CATCH_INCLUDE_DIR} ${COMMON_INCLUDES})
# Includes Spdlog in the project:
add_subdirectory(${EXT_PROJECTS_DIR}/spdlog)
include_directories(${SPDLOG_INCLUDE_DIR} ${COMMON_INCLUDES})
#
enable_testing(true)  # Enables unit-testing
