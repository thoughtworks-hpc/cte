cmake_minimum_required(VERSION 3.10)
project(trade_persistence_tool)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(protobuf REQUIRED)
find_package(protoc REQUIRED)
find_package(grpc REQUIRED)

add_executable(${PROJECT_NAME}_server test/fake_server_starter.cc test/fake_server.cc)
target_link_libraries(${PROJECT_NAME}_server  common_grpc grpc::grpc)

add_executable(${PROJECT_NAME}_main src/main.cc src/trade_persist_influxdb.cc src/trade_persistence_client.cc )
target_link_libraries(${PROJECT_NAME}_main  cdcf::cdcf common_grpc grpc::grpc )

aux_source_directory(test ALL_SOURCES)
set(TEST_SOURCES ${ALL_SOURCES})
list(FILTER TEST_SOURCES EXCLUDE REGEX "server_starter.cc$")

if (NOT "${TEST_SOURCES}" STREQUAL "")
    add_executable(${PROJECT_NAME}_test  ${TEST_SOURCES} src/trade_persist_influxdb.cc src/trade_persistence_client.cc)
    find_package(GTest REQUIRED)
    target_link_libraries(${PROJECT_NAME}_test common common_grpc GTest::GTest protobuf::protobuf grpc::grpc)
    #    protobuf_generate(LANGUAGE cpp TARGET ${PROJECT_NAME}_test)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
    set_tests_properties(${PROJECT_NAME}_test PROPERTIES TIMEOUT 60)
endif ()