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
#include "../include/order_store_influxdb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

template <typename Out>
void split(const std::string& s, char delimiter, Out result) {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delimiter)) {
    *result++ = item;
  }
}

void RunServer(const std::string& order_manager_address,
               const std::string& match_engine_main_address,
               std::vector<const std::string>& match_engine_request_addresses) {
  assert(!order_manager_address.empty());
  assert(!match_engine_main_address.empty());

  std::shared_ptr<Channel> main_channel;
  std::vector<const std::shared_ptr<Channel>> request_channels;

  main_channel = grpc::CreateChannel(match_engine_main_address,
                                     grpc::InsecureChannelCredentials());

  if (!match_engine_request_addresses.empty()) {
    for (const auto& address : match_engine_request_addresses) {
      request_channels.push_back(
          grpc::CreateChannel(address, grpc::InsecureChannelCredentials()));
    }
  }

  auto order_store = std::make_shared<OrderStoreInfluxDB>();

  OrderManagerImpl service(order_store, main_channel, request_channels);

  ServerBuilder builder;
  builder.AddListeningPort(order_manager_address,
                           grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << order_manager_address << std::endl;
  server->Wait();
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "usage: order_manager localhost:50051 localhost:4770 "
                 "localhost:4771,localhost:4772,localhost:4773"
              << std::endl;
    return 0;
  }

  std::string order_manager_address = argv[1];
  std::string match_engine_main_address = argv[2];
  std::vector<const std::string> match_engine_request_addresses;

  if (argc == 4) {
    split(argv[3], ',', std::back_inserter(match_engine_request_addresses));
  }

  RunServer(order_manager_address, match_engine_main_address,
            match_engine_request_addresses);

  return 0;
}