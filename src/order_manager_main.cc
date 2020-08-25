/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

//#include "./order_manager.grpc.pb.h"
#include "./match_engine.grpc.pb.h"
#include "./order_manager.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  OrderManagerImpl service(grpc::CreateChannel(
      "localhost:50052", grpc::InsecureChannelCredentials()));

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main() { RunServer(); }