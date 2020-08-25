CMAKE_MINIMUM_REQUIRED(VERSION 3.5.2 FATAL_ERROR)
project(common)

find_package(protobuf REQUIRED)
find_package(protoc REQUIRED)
find_package(grpc REQUIRED)

set(PB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/protobuf_gen")
set(PB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/protobuf")

set(NODE_MONITOR_PROTO_SRCS "${PB_DIR}/match_engine.pb.cc")
set(NODE_MONITOR_PROTO_HDRS "${PB_DIR}/match_engine.pb.h")
set(NODE_MONITOR_GRPC_SRCS "${PB_DIR}/match_engine.grpc.pb.cc")
set(NODE_MONITOR_GRPC_HDRS "${PB_DIR}/match_engine.grpc.pb.h")

execute_process(COMMAND mkdir -p ${PB_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(GRPC_CPP_PLUGIN ${grpc_LIB_DIRS}/../bin/grpc_cpp_plugin)

add_custom_command(
        OUTPUT "${NODE_MONITOR_PROTO_SRCS}" "${NODE_MONITOR_PROTO_HDRS}" "${NODE_MONITOR_GRPC_SRCS}" "${NODE_MONITOR_GRPC_HDRS}"
        COMMAND $<TARGET_FILE:protobuf::protoc>
        ARGS --grpc_out "${PB_DIR}"
        --cpp_out "${PB_DIR}"
        -I "${PB_SOURCE_DIR}"
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        "${PB_SOURCE_DIR}/match_engine.proto"
        DEPENDS "${PB_SOURCE_DIR}/match_engine.proto")


list(APPEND PB_ALL_SOURCES ${NODE_MONITOR_PROTO_SRCS} ${NODE_MONITOR_GRPC_SRCS})


add_library(${PROJECT_NAME}_grpc ${PB_ALL_SOURCES})
target_link_libraries(${PROJECT_NAME}_grpc grpc::grpc)

