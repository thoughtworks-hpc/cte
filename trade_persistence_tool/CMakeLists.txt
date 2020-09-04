cmake_minimum_required(VERSION 3.10)
project(trade_persistence_tool)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(protobuf REQUIRED)
find_package(protoc REQUIRED)
find_package(grpc REQUIRED)

add_executable(${PROJECT_NAME}_server test/server_starter.cc test/server.cc)
target_link_libraries(${PROJECT_NAME}_server  common_grpc grpc::grpc)

add_executable(${PROJECT_NAME}_client test/client.cc)
target_link_libraries(${PROJECT_NAME}_client  common_grpc grpc::grpc)

add_executable(${PROJECT_NAME}_main src/main.cc src/trade_persist_influxdb.cc src/trade_persistence_client.cc)
target_link_libraries(${PROJECT_NAME}_main  common_grpc grpc::grpc)