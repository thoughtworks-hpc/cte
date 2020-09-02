cmake_minimum_required(VERSION 3.10)
project(request_generator)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(protobuf REQUIRED)
find_package(protoc REQUIRED)
find_package(grpc REQUIRED)

message("---------------------------------------")
message(${CMAKE_CURRENT_SOURCE_DIR})
message(${CMAKE_BINARY_DIR})
message(${CMAKE_CURRENT_BINARY_DIR})
message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR})
#message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
message("---------------------------------------")

set(PB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/protobuf_gen")
set(PB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/protobuf")

set(NODE_MONITOR_PROTO_SRCS "${PB_DIR}/order.pb.cc")
set(NODE_MONITOR_PROTO_HDRS "${PB_DIR}/order.pb.h")
set(NODE_MONITOR_GRPC_SRCS "${PB_DIR}/order.grpc.pb.cc")
set(NODE_MONITOR_GRPC_HDRS "${PB_DIR}/order.grpc.pb.h")

execute_process(COMMAND mkdir -p ${PB_DIR} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(GRPC_CPP_PLUGIN ${grpc_LIB_DIRS}/../bin/grpc_cpp_plugin)

add_custom_command(
        OUTPUT "${NODE_MONITOR_PROTO_SRCS}" "${NODE_MONITOR_PROTO_HDRS}" "${NODE_MONITOR_GRPC_SRCS}" "${NODE_MONITOR_GRPC_HDRS}"
        COMMAND $<TARGET_FILE:protobuf::protoc>
        ARGS --grpc_out "${PB_DIR}"
        --cpp_out "${PB_DIR}"
        -I "${PB_SOURCE_DIR}"
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        "${PB_SOURCE_DIR}/order.proto"
        DEPENDS "${PB_SOURCE_DIR}/order.proto")


list(APPEND PB_ALL_SOURCES ${NODE_MONITOR_PROTO_SRCS} ${NODE_MONITOR_GRPC_SRCS})
add_library(${PROJECT_NAME}_grpc ${PB_ALL_SOURCES})

add_executable(${PROJECT_NAME}_server test/server_starter.cc test/server.cc)
target_link_libraries(${PROJECT_NAME}_server ${PROJECT_NAME}_grpc grpc::grpc)


add_executable(${PROJECT_NAME}_main src/main.cc src/generator.cc src/Influxdb_reader.cc)
target_link_libraries(${PROJECT_NAME}_main ${PROJECT_NAME}_grpc grpc::grpc)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/request_generator_config.json
        DESTINATION ${CMAKE_BINARY_DIR}/bin
        )
aux_source_directory(test ALL_SOURCES)
set(TEST_SOURCES ${ALL_SOURCES})
list(FILTER TEST_SOURCES EXCLUDE REGEX "server_starter.cc$")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/request_generator_config_test.json
        DESTINATION ${CMAKE_BINARY_DIR}/bin
        )
if (NOT "${TEST_SOURCES}" STREQUAL "")
    add_executable(${PROJECT_NAME}_test  ${TEST_SOURCES} src/generator.cc test/server.cc)
    find_package(GTest REQUIRED)
    target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME}_grpc GTest::GTest grpc::grpc)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
    set_tests_properties(${PROJECT_NAME}_test PROPERTIES TIMEOUT 60)
endif ()