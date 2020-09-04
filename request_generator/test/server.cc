/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "test/server.h"

#include <grpcpp/server_builder.h>

#include <iostream>
#include <string>
#include <utility>

::grpc::Status OrderManagerImpl::PlaceOrder(
    ::grpc::ServerContext* context, const ::order_manager_proto::Order* request,
    ::order_manager_proto::Reply* response) {
  std::cout << "Receive #" << ++count_ << " request from client" << std::endl;
  //    std::cout << "symbol: " << request->symbol() << std::endl;
  //    std::cout << "user_id: " << request->user_id() << std::endl;
  //    std::cout << "trading_side: " << request->trading_side() << std::endl;
  //    std::cout << "amount: " << request->amount() << std::endl;
  //    std::cout << "price: " << request->price() << std::endl;
  //    std::cout << "-------------------------------" << std::endl;

  response->set_error_code(order_manager_proto::ErrorCode::SUCCESS);
  std::string response_message =
      "#" + std::to_string(count_) + " Response form server";
  response->set_message(response_message);

  return grpc::Status::OK;
}

void OrderManagerImpl::Run() {
  std::string server_address("0.0.0.0:" + port_);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(this);
  server_ = builder.BuildAndStart();
  std::cout << "grpc server is listening " << port_ << std::endl;
}

void OrderManagerImpl::RunWithWait() {
  Run();
  server_->Wait();
}

void writeData(order_manager_proto::Order* order) { order->set_amount(10); }
