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
#include "../../common/protobuf_gen/order_manager.grpc.pb.h"
#include "../../common/protobuf_gen/order_manager.pb.h"
#include "./config_manager.h"
#include "Influxdb_reader.h"
#include "src/config.h"
#include "src/generator.h"

int main(int argc, char *argv[]) {
  //  char **config_file_path = new char *[2] { INI_FILE_PARAMETER };
  //  request_generator::Config config(config_file_path[0]);
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
        printf("Unknown option:  %c\n", static_cast<char>(optopt));
        exit(1);
    }
  }

  auto database = new Influxdb_reader();
  Generator generator(num_of_threads, num_of_requests, grcp_ip_address,
                      db_host_address, db_port, database);
  generator.Start();
}
