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

# Compile main.cpp
add_executable(project src/main.cpp)

# Link the dependencies with our binary
message("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@")
message("${CONAN_LIBS}")

target_link_libraries(project ${CONAN_LIBS} caf::caf cdcf::cdcf)

add_subdirectory(common)
add_subdirectory(influxdb_usa_demo)
add_subdirectory(match_engine)



