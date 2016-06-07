cmake_minimum_required(VERSION 2.8.8)
project(yijian)

add_executable(a.out main.cxx)

set(CMAKE_BUILD_TYPE "Debug")
set(CXX_FLAGS "-std=c++11 -pthread")
message(STATUS ${CXX_FLAGS})
#set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "${CXX_FLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CXX_FLAGS} -O3 -Wall")

add_subdirectory(yijian)
include_directories(yijian)
#target_link_libraries(a.out yijian)

# catch
set(EXT_PROJECTS_DIR submodules)
add_subdirectory(${EXT_PROJECTS_DIR}/Catch)
include_directories(submodules/Catch/include)
enable_testing(true)  # Enables unit-testing

