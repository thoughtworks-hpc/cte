/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include <grpcpp/create_channel.h>
#include <grpcpp/impl/codegen/client_context.h>

#include <iostream>
#include <queue>

#include "../../common/include/influxdb.hpp"
#include "../../common/include/json.hpp"
#include "../protobuf_gen/order.grpc.pb.h"
#include "../protobuf_gen/order.pb.h"
#include "src/generator.h"

int Generator::requests_count_ = 0;
std::mutex Generator::mutex_requests_count_;

int main(int argc, char* argv[]) {
  std::queue<order_manager_proto::Order> orders;
  for (int i = 0; i < argc; i++) {
    std::cout << argv[i] << " " << std::endl;
  }

  Generator generator(4, 10000);
  generator.Start();
  std::cout << "Total requests: " << generator.getRequestsCount() << std::endl;
}
