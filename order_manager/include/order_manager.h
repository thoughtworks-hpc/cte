/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_ORDER_MANAGER_H_
#define ORDER_MANAGER_INCLUDE_ORDER_MANAGER_H_

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <memory>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"
#include "../../common/protobuf_gen/order_manager.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

class OrderManagerImpl final
    : public order_manager_proto::OrderManager::Service {
 public:
  explicit OrderManagerImpl(const std::shared_ptr<Channel>& channel)
      : order_id_(0),
        stub_(::match_engine_proto::TradingEngine::NewStub(channel)) {}

  ::grpc::Status PlaceOrder(::grpc::ServerContext* context,
                            const ::order_manager_proto::Order* request,
                            ::order_manager_proto::Reply* response) override;

 private:
  std::atomic<std::int64_t> order_id_;
  std::unique_ptr<::match_engine_proto::TradingEngine::Stub> stub_;

  void SubscribeMatchResult() {}
};

#endif  // ORDER_MANAGER_INCLUDE_ORDER_MANAGER_H_
