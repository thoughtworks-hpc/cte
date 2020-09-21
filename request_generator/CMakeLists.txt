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

add_executable(${PROJECT_NAME}_server test/server_starter.cc test/server.cc)
target_link_libraries(${PROJECT_NAME}_server common common_grpc grpc::grpc)

add_executable(${PROJECT_NAME}_main src/main.cc src/generator.cc src/Influxdb_reader.cc src/config.cc)
target_link_libraries(${PROJECT_NAME}_main common common_grpc grpc::grpc)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/request_generator_config.json
        DESTINATION ${CMAKE_BINARY_DIR}/bin
        )

aux_source_directory(test ALL_SOURCES)
set(TEST_SOURCES ${ALL_SOURCES} test/config_manager_test.h)
list(FILTER TEST_SOURCES EXCLUDE REGEX "server_starter.cc$")
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test/request_generator_config_test.json
#        DESTINATION ${CMAKE_BINARY_DIR}/bin
#        )

# list(APPEND TEST_SOURCES protobuf/order.proto)

set(TEST_INI_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/test/request_generator_config_test.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/config_manager_test.h.in ${CMAKE_CURRENT_SOURCE_DIR}/test/config_manager_test.h)

if (NOT "${TEST_SOURCES}" STREQUAL "")
    add_executable(${PROJECT_NAME}_test  ${TEST_SOURCES} src/generator.cc test/server.cc src/config.cc)
    find_package(GTest REQUIRED)
    target_link_libraries(${PROJECT_NAME}_test common common_grpc GTest::GTest protobuf::protobuf grpc::grpc)
#    protobuf_generate(LANGUAGE cpp TARGET ${PROJECT_NAME}_test)
    add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)
    set_tests_properties(${PROJECT_NAME}_test PROPERTIES TIMEOUT 60)
endif ()