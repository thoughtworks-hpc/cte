# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.10)

# Name of the project
project(Project)
set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/build)
if ("${CONAN_COMPILER}" STREQUAL "gcc")
    set(CMAKE_C_FLAGS ${CONAN_C_FLAGS})
    set(CMAKE_CXX_FLAGS ${CONAN_CXX_FLAGS})
    set(CMAKE_SHARED_LINKER_FLAGS "${CONAN_SHARED_LINKER_FLAGS}")
endif ()

find_package(caf REQUIRED)
find_package(cdcf REQUIRED)
find_package(protobuf REQUIRED)
find_package(protoc REQUIRED)
find_package(grpc REQUIRED)

message("----------------------------------")
message("${caf_LIBRARIES}")
message("${caf_LIBS}")
message("----------------------------------")
message("${cdcf_LIBRARIES}")
message("${cdcf_LIBS}")
message("----------------------------------")

# Include Conan dependencies
message("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
message("${CMAKE_BINARY_DIR}")
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

# This will setup conan environment. Without this, including our dependencies
# would fail
conan_basic_setup()

if (WIN32)
    set(GRPC_CPP_PLUGIN ${grpc_LIB_DIRS}/../bin/grpc_cpp_plugin.exe)
else ()
    set(GRPC_CPP_PLUGIN ${grpc_LIB_DIRS}/../bin/grpc_cpp_plugin)
endif ()

set(PB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(ORDER_MANAGER_PROTO_SRCS "${PB_DIR}/order_manager.pb.cc")
set(ORDER_MANAGER_PROTO_HDRS "${PB_DIR}/order_manager.pb.h")
set(ORDER_MANAGER_GRPC_SRCS "${PB_DIR}/order_manager.grpc.pb.cc")
set(ORDER_MANAGER_GRPC_HDRS "${PB_DIR}/order_manager.grpc.pb.h")
add_custom_command(
        OUTPUT "${ORDER_MANAGER_PROTO_SRCS}" "${ORDER_MANAGER_PROTO_HDRS}" "${ORDER_MANAGER_GRPC_SRCS}" "${ORDER_MANAGER_GRPC_HDRS}"
        COMMAND $<TARGET_FILE:protobuf::protoc>
        ARGS --grpc_out "${PB_DIR}"
        --cpp_out "${PB_DIR}"
        -I "${CMAKE_CURRENT_SOURCE_DIR}/common/protobuf"
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        "${CMAKE_CURRENT_SOURCE_DIR}/common/protobuf/order_manager.proto"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/common/protobuf/order_manager.proto")
#list(APPEND ALL_SOURCES ${ORDER_MANAGER_PROTO_SRCS} ${ORDER_MANAGER_GRPC_SRCS})

set(MATCH_ENGINE_PROTO_SRCS "${PB_DIR}/match_engine.pb.cc")
set(MATCH_ENGINE_PROTO_HDRS "${PB_DIR}/match_engine.pb.h")
set(MATCH_ENGINE_GRPC_SRCS "${PB_DIR}/match_engine.grpc.pb.cc")
set(MATCH_ENGINE_GRPC_HDRS "${PB_DIR}/match_engine.grpc.pb.h")
add_custom_command(
        OUTPUT "${MATCH_ENGINE_PROTO_SRCS}" "${MATCH_ENGINE_PROTO_HDRS}" "${MATCH_ENGINE_GRPC_SRCS}" "${MATCH_ENGINE_GRPC_HDRS}"
        COMMAND $<TARGET_FILE:protobuf::protoc>
        ARGS --grpc_out "${PB_DIR}"
        --cpp_out "${PB_DIR}"
        -I "${CMAKE_CURRENT_SOURCE_DIR}/common/protobuf"
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
        "${CMAKE_CURRENT_SOURCE_DIR}/common/protobuf/match_engine.proto"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/common/protobuf/match_engine.proto")
#list(APPEND ALL_SOURCES ${ORDER_MANAGER_PROTO_SRCS} ${ORDER_MANAGER_GRPC_SRCS})

# Compile main.cpp

add_executable(project src/main.cpp)
add_executable(order_manager src/order_manager_main.cc src/order_manager.cc ${ORDER_MANAGER_PROTO_SRCS} ${ORDER_MANAGER_GRPC_SRCS} ${MATCH_ENGINE_PROTO_SRCS} ${MATCH_ENGINE_GRPC_SRCS})

# Link the dependencies with our binary
message("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
message("${CONAN_LIBS}")

target_link_libraries(project ${CONAN_LIBS} caf::caf cdcf::cdcf protobuf::protobuf grpc::grpc common)
target_link_libraries(order_manager ${CONAN_LIBS} caf::caf cdcf::cdcf protobuf::protobuf grpc::grpc common)
#protobuf_generate(LANGUAGE cpp TARGET project)

add_subdirectory(influxdb_usa_demo)



