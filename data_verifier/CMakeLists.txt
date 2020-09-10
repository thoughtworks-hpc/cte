cmake_minimum_required(VERSION 3.10)
project(data_verifier)

add_executable(${PROJECT_NAME} src/data_verifier_main.cc src/data_source_influxdb.cc src/data_verifier.cc)
target_link_libraries(${PROJECT_NAME} common common_grpc cdcf::cdcf)

add_executable(${PROJECT_NAME}_test src/data_verifier_test.cc src/data_verifier.cc src/data_source_influxdb.cc src/data_source_mock.cc)
find_package(GTest REQUIRED)
target_link_libraries(${PROJECT_NAME}_test GTest::GTest common common_grpc cdcf::cdcf)
enable_testing()
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)