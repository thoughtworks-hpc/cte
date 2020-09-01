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
#include "InfluxdbReader.h"
#include "src/config.h"
#include "src/generator.h"

int main(int argc, char* argv[]) {
  request_generator::Config config("request_generator_config.json");
  int num_of_threads = config.default_num_of_threads_;
  int num_of_requests = config.default_num_of_requests_;
  std::string db_host_address = config.default_db_host_address_;
  std::string db_port = config.default_db_port_;
  std::vector<ip_address> grcp_ip_address = config.grcp_ip_address_;

  int option;
  std::queue<order_manager_proto::Order> orders;
  while ((option = getopt(argc, argv, "t:n:")) != -1) {
    switch (option) {
      case 't':
        num_of_threads = std::stoi(optarg);
        break;
      case 'n':
        num_of_requests = std::stoi(optarg);
        break;
      case '?':
        printf("Unknown option: %c\n", static_cast<char>(optopt));
        exit(1);
    }
  }

  auto database = new InfluxdbReader();
  Generator generator(num_of_threads, num_of_requests, grcp_ip_address,
                      db_host_address, db_port, database);
  generator.Start();

  const std::vector<int> count_each_server = Generator::getCountEachServer();
  for (int i = 0; i < count_each_server.size(); i++) {
    std::cout << "Send " << count_each_server[i] << " requests to "
              << grcp_ip_address[i].ip_ << ":" << grcp_ip_address[i].port_
              << std::endl;
  }
  std::cout << "Total requests: " << Generator::GetRequestsCount() << std::endl;
}
