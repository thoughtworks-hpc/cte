cmake_minimum_required(VERSION 3.10)
project(order_manager)

conan_basic_setup()

add_executable(order_manager src/order_manager.cc src/main.cc)

target_link_libraries(order_manager common caf::caf cdcf::cdcf common_grpc ${CONAN_LIBS})