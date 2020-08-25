CMAKE_MINIMUM_REQUIRED(VERSION 3.5.2 FATAL_ERROR)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR" )
project(order_generator)

include_directories(./include)

add_executable(order_generator src/order_generator.cc)

find_package(GTest REQUIRED)
enable_testing()
add_executable(${PROJECT_NAME}_test src/order_generator_test.cc)
target_link_libraries(${PROJECT_NAME}_test GTest::GTest)
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test)



