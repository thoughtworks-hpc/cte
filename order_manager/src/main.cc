/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "../include/order_manager.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

template <typename Out>
void split(const std::string& s, char delim, Out result) {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim)) {
    *result++ = item;
  }
}

void RunServer(const std::string& order_manager_port,
               const std::string& match_engine_main_port,
               std::vector<const std::string>& match_engine_request_port) {
  assert(!order_manager_port.empty());
  assert(!match_engine_main_port.empty());

  std::string server_address(order_manager_port);
  std::shared_ptr<Channel> main_channel;
  std::vector<const std::shared_ptr<Channel>> request_channels;

  main_channel = grpc::CreateChannel(match_engine_main_port,
                                     grpc::InsecureChannelCredentials());

  if (!match_engine_request_port.empty()) {
    for (const auto& port : match_engine_request_port) {
      request_channels.push_back(
          grpc::CreateChannel(port, grpc::InsecureChannelCredentials()));
    }
  }

  OrderManagerImpl service(main_channel, request_channels);

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "usage: order_manager localhost:50051 localhost:4770 "
                 "localhost:4771,localhost:4772,localhost:4773"
              << std::endl;
  }

  std::string order_manager_port = argv[1];
  std::string match_engine_main_port = argv[2];
  std::vector<const std::string> match_engine_request_port;

  if (argc == 4) {
    split(argv[3], ',', std::back_inserter(match_engine_request_port));
  }

  RunServer(order_manager_port, match_engine_main_port,
            match_engine_request_port);
}