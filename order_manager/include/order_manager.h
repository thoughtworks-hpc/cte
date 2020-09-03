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
#include <string>
#include <unordered_map>
#include <vector>

#include "../../common/protobuf_gen/match_engine.grpc.pb.h"
#include "../../common/protobuf_gen/order_manager.grpc.pb.h"
#include "./match_engine_stub.h"
#include "./order_store.h"

class OrderManagerService final
    : public order_manager_proto::OrderManager::Service {
 public:
  explicit OrderManagerService(
      std::shared_ptr<OrderStore> order_store,
      std::shared_ptr<MatchEngineStub> match_engine_stub);

  ::grpc::Status PlaceOrder(::grpc::ServerContext* context,
                            const ::order_manager_proto::Order* request,
                            ::order_manager_proto::Reply* response) override;

 private:
  class OrderStatus {
   public:
    match_engine_proto::Order order;
    int32_t concluded_amount;
  };

  void BuildMatchEngineOrder(const order_manager_proto::Order& order_in_request,
                             match_engine_proto::Order& order);
  void SaveOrderStatus(const match_engine_proto::Order& order);
  void HandleMatchResult(const ::match_engine_proto::Trade& trade);

  std::atomic<int64_t> order_id_;
  mutable std::mutex mutex_;
  std::unordered_map<int64_t, OrderStatus> order_id_to_order_status_;

  std::shared_ptr<OrderStore> order_store_;

  std::shared_ptr<MatchEngineStub> match_engine_stub_;
};

#endif  // ORDER_MANAGER_INCLUDE_ORDER_MANAGER_H_
