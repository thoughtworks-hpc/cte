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
#include <mutex>

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
  explicit OrderManagerImpl(const std::shared_ptr<Channel>& channel);

  ::grpc::Status PlaceOrder(::grpc::ServerContext* context,
                            const ::order_manager_proto::Order* request,
                            ::order_manager_proto::Reply* response) override;

 private:
  class OrderStatus {
   public:
    match_engine_proto::Order order;
    int32_t transaction_amount;
  };

  std::atomic<int64_t> order_id_;
  std::unique_ptr<::match_engine_proto::TradingEngine::Stub> stub_;
  mutable std::mutex mutex_;
  std::unordered_map<int64_t, OrderStatus> order_id_to_order_status_;

  static void BuildMatchEngineOrder(
      const order_manager_proto::Order& order_in_request, int64_t order_id,
      int64_t nanoseconds_since_epoch, match_engine_proto::Order& order);
  void SaveOrderStatus(const match_engine_proto::Order& order);
  static int PersistOrder(const match_engine_proto::Order& order,
                          std::string status);

  void SubscribeMatchResult();
};

#endif  // ORDER_MANAGER_INCLUDE_ORDER_MANAGER_H_
