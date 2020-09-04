cmake_minimum_required(VERSION 3.10)

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

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

conan_basic_setup()

include(CTest)
add_subdirectory(common)
add_subdirectory(influxdb_usa_demo)
add_subdirectory(match_engine)
add_subdirectory(order_generator)
add_subdirectory(order_manager)
add_subdirectory(request_generator)
add_subdirectory(trade_persistence_tool)