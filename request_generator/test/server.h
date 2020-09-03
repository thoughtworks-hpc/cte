/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_TEST_SERVER_H_
#define REQUEST_GENERATOR_TEST_SERVER_H_

#include <grpcpp/server_builder.h>

#include <memory>
#include <string>
#include <utility>

#include "protobuf_gen/order.grpc.pb.h"
#include "protobuf_gen/order.pb.h"

class OrderManagerImpl final
    : public order_manager_proto::OrderManager::Service {
 public:
  std::string port_;
  std::atomic_int32_t count_ = 0;
  explicit OrderManagerImpl(std::string port) : port_(std::move(port)) {}

  grpc::Status PlaceOrder(grpc::ServerContext* context,
                          const order_manager_proto::Order* request,
                          order_manager_proto::Reply* response);
  void Run();
  void RunWithWait();

 private:
  std::unique_ptr<grpc::Server> server_;
};

#endif  // REQUEST_GENERATOR_TEST_SERVER_H_
