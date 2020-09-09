/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <grpcpp/create_channel.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <queue>

#include "../../common/include/json.hpp"
#include "../../common/protobuf_gen/order_manager.grpc.pb.h"
#include "../../common/protobuf_gen/order_manager.pb.h"
#include "include/Influxdb_reader.h"
#include "include/config.h"
#include "include/generator.h"

int main(int argc, char *argv[]) {
  request_generator::Config config(argc, argv);

  influxdb_cpp::server_info si(
      config.database_ip_, std::stoi(config.database_port_),
      config.database_name_, config.database_user_name_,
      config.database_password_);
  auto database = new InfluxdbReader(si);
  Generator generator(config.num_of_threads_, config.num_of_requests_,
                      config.grcp_ip_address_, database);

  generator.Start();
}
