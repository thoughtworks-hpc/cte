/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_ORDER_MANAGER_H
#define CTE_ORDER_MANAGER_H

#include "../../common/protobuf_gen/order_manager.grpc.pb.h"
#include "../../common/protobuf_gen/match_engine.grpc.pb.h"
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

class OrderManagerImpl final : public order_manager_proto::OrderManager::Service {
 public:
  explicit OrderManagerImpl(const std::shared_ptr<Channel>& channel) : stub_(::match_engine_proto::TradingEngine::NewStub(channel)) {

  }

  ::grpc::Status PlaceOrder(::grpc::ServerContext* context,
                            const ::order_manager_proto::Order* request, ::order_manager_proto::Reply* response) override;

 private:
    std::unique_ptr<::match_engine_proto::TradingEngine::Stub> stub_;
};

#endif  // CTE_ORDER_MANAGER_H
