/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <string>

#include "../include/fake_match_engine.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

void RunServer(const std::string& server_port) {
  std::string server_address(server_port);
  FakeMatchEngineService service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "usage: fake_match_engine localhost:50052" << std::endl;
  }

  RunServer(argv[1]);
}
