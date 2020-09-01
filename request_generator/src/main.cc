/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <grpcpp/create_channel.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <queue>

#include "../../common/include/influxdb.hpp"
#include "../../common/include/json.hpp"
#include "../protobuf_gen/order.grpc.pb.h"
#include "../protobuf_gen/order.pb.h"
#include "src/generator.h"

int main(int argc, char* argv[]) {
  nlohmann::json initial_prices_config;
  std::ifstream input("request_generator_config.json");
  input >> initial_prices_config;
  // std::cout << initial_prices_config << std::endl;
  int default_num_of_threads = initial_prices_config["default_num_of_threads"];
  int default_num_of_requests =
      initial_prices_config["default_num_of_requests"];
  std::string default_db_host_address =
      initial_prices_config["default_db_host_address"];
  std::string default_db_port = initial_prices_config["default_db_port"];

  std::vector<ip_address> grcp_ip_address;
  for (int i = 0; i < initial_prices_config["grpc_server"].size(); i++) {
    grcp_ip_address.emplace_back(
        static_cast<std::string>(initial_prices_config["grpc_server"][i]["ip"]),
        static_cast<std::string>(
            initial_prices_config["grpc_server"][i]["port"]));
  }

  int option;
  std::queue<order_manager_proto::Order> orders;
  while ((option = getopt(argc, argv, "t:n:")) != -1) {
    switch (option) {
      case 't':
        default_num_of_threads = std::stoi(optarg);
        break;
      case 'n':
        default_num_of_requests = std::stoi(optarg);
        break;
      case '?':
        printf("Unknown option: %c\n", static_cast<char>(optopt));
        exit(1);
    }
  }

  Generator generator(default_num_of_threads, default_num_of_requests,
                      grcp_ip_address, default_db_host_address,
                      default_db_port);
  generator.Start();

  const std::vector<int> count_each_server = Generator::getCountEachServer();
  for (int i = 0; i < count_each_server.size(); i++) {
    std::cout << "Send " << count_each_server[i] << " requests to "
              << grcp_ip_address[i].ip_ << ":" << grcp_ip_address[i].port_
              << std::endl;
  }
  std::cout << "Total requests: " << Generator::GetRequestsCount() << std::endl;
}
