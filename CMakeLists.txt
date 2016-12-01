cmake_minimum_required(VERSION 2.8.8)
project(yijian)
add_executable(a.out main.cxx ./yijian/macro.cxx ./yijian/mongo.cxx ./yijian/protofiles/chat_message.pb.cc ./yijian/lib_client.cxx ./yijian/buffer.cxx)
#add_executable(server.out server.cxx ./yijian/macro.cxx ./yijian/threads_work.cxx ./yijian/libev_server.cxx ./yijian/mongo.cxx ./yijian/server_msg_typemap.cxx ./yijian/protofiles/chat_message.pb.cc ./yijian/lib_client.cxx ./yijian/pinglist.cxx ./yijian/buffer.cxx)
#add_executable(client.out client.cxx)
add_executable(im_server.out im_server.cxx ./yijian/macro.cxx ./yijian/threads_work.cxx ./yijian/libev_server.cxx ./yijian/mongo.cxx ./yijian/server_msg_typemap.cxx ./yijian/protofiles/chat_message.pb.cc ./yijian/pinglist.cxx ./yijian/buffer.cxx)

set(CMAKE_BUILD_TYPE "Debug")
#c++ --std=c++11 test.cpp -o test $(pkg-config --cflags --libs libmongocxx)
#-I/usr/include/mongocxx/v_noabi -I/usr/include/libmongoc-1.0 -I/usr/include/bsoncxx/v_noabi -I/usr/include/libbson-1.0 -lmongocxx -lbsoncxx
set(CXX_MONGO_FLAGS "-I/usr/include/mongocxx/v_noabi -I/usr/include/libmongoc-1.0 -I/usr/include/bsoncxx/v_noabi -I/usr/include/libbson-1.0 -lmongocxx -lbsoncxx") 
#execute_process(COMMAND bash "-c" "pkg-config --cflags --libs protobuf" OUTPUT_VARIABLE CXX_PROTO_FLAGS)
#execute_process(COMMAND bash "-c" "pkg-config --cflags --libs libmongocxx" OUTPUT_VARIABLE CXX_MONGO_FLAGS)
set(CXX_PROTO_FLAGS "-pthread -lprotobuf -pthread -lpthread")
set(CXX_FLAGS "-std=c++14 -D_GLIBCXX_USE_CXX11_ABI=1 -DYILOG_ON=1 -lev -pthread -lboost_system ${CXX_MONGO_FLAGS} ${CXX_PROTO_FLAGS}")
message(STATUS ${CXX_FLAGS})
#set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS_DEBUG "${CXX_FLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CXX_FLAGS} -O3 -Wall")

add_subdirectory(yijian)
include_directories(yijian)
#target_link_libraries(a.out yijian)
#target_link_libraries(server.out yijian)
#target_link_libraries(client.out yijian)

#
set(EXT_PROJECTS_DIR externals)
# Includes Catch in the project:
add_subdirectory(${EXT_PROJECTS_DIR}/catch)
include_directories(${CATCH_INCLUDE_DIR} ${COMMON_INCLUDES})
# Includes Spdlog in the project:
add_subdirectory(${EXT_PROJECTS_DIR}/spdlog)
include_directories(${SPDLOG_INCLUDE_DIR} ${COMMON_INCLUDES})
# Enables unit-testing
enable_testing(true)  
