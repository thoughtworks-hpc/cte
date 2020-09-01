cmake_minimum_required(VERSION 3.10)
project(match_engine)

add_executable(match_engine_server src/match_actor.cc
        src/match_grpc_server.cc
        src/match_engine_grpc_impl.cc
        src/symbol_id_router.cc
        src/match_engine_cluster.cc
        src/match_result_sender_actor.cc)

target_link_libraries(match_engine_server common caf::caf cdcf::cdcf common_grpc)