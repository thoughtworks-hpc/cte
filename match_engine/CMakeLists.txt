cmake_minimum_required(VERSION 3.10)
project(match_engine)

add_executable(${PROJECT_NAME}_server src/match_actor.cc
        src/match_grpc_server.cc
        src/match_engine_grpc_impl.cc
        src/symbol_id_router.cc
        src/match_engine_cluster.cc
        src/match_result_sender_actor.cc)

target_link_libraries(${PROJECT_NAME}_server common caf::caf cdcf::cdcf common_grpc)


add_executable(match_actor_test src/match_actor_test.cc src/match_actor.cc src/match_result_sender_actor.cc)
target_link_libraries(match_actor_test common caf::caf cdcf::cdcf common_grpc)
add_test(NAME match_actor_test COMMAND match_actor_test)

add_executable(${PROJECT_NAME}_grpc_test src/match_engine_grpc_test.cc src/match_engine_grpc_impl.cc src/match_actor.cc src/match_result_sender_actor.cc src/match_engine_cluster.cc src/symbol_id_router.cc)
target_link_libraries(${PROJECT_NAME}_grpc_test common caf::caf cdcf::cdcf common_grpc GTest::GTest)
add_test(NAME ${PROJECT_NAME}_grpc_test COMMAND ${PROJECT_NAME}_grpc_test)