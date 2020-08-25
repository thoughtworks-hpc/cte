cmake_minimum_required(VERSION 3.10)
project(match_engine)

add_executable(match_engine_server src/match_actor.cc src/match_grpc_server.cc)

target_link_libraries(match_engine_server caf::caf cdcf::cdcf)