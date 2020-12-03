cmake_minimum_required(VERSION 3.10)
project(order_manager)

add_executable(${PROJECT_NAME} src/order_manager.cc src/order_store_influxdb.cc src/match_engine_stub_grpc.cc src/order_manager_main.cc ../common/src/database_interface.cc)
add_executable(fake_match_engine src/fake_match_engine.cc src/fake_match_engine_main.cc)

target_link_libraries(${PROJECT_NAME} common common_grpc cdcf::cdcf)
target_link_libraries(fake_match_engine common common_grpc)

add_executable(${PROJECT_NAME}_test src/order_manager_test.cc src/order_manager.cc src/order_store_mock.cc src/match_engine_stub_fake.cc ../common/src/database_interface.cc)
find_package(GTest REQUIRED)
target_link_libraries(${PROJECT_NAME}_test GTest::GTest common common_grpc cdcf::cdcf)
enable_testing()
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/order_manager_record_config.json
        DESTINATION ${CMAKE_BINARY_DIR}/bin
        )