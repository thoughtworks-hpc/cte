/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_ORDER_MANAGER_H
#define CTE_ORDER_MANAGER_H

#include "./order_manager.grpc.pb.h"
#include "./match_engine.grpc.pb.h"
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

class OrderManagerImpl final : public order_manager::OrderManager::Service {
 public:
  explicit OrderManagerImpl(const std::shared_ptr<Channel>& channel) : stub_(::match_engine::TradingEngine::NewStub(channel)) {

  }

  ::grpc::Status PlaceOrder(::grpc::ServerContext* context,
                            const ::order_manager::Order* request, ::order_manager::Reply* response) override;

 private:
    std::unique_ptr<::match_engine::TradingEngine::Stub> stub_;
};

#endif  // CTE_ORDER_MANAGER_H
