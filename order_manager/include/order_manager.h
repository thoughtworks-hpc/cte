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

#include <chrono>
#include <memory>
#include <mutex>
#include <shared_mutex>
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
  ::grpc::Status StartEndManager(
      ::grpc::ServerContext* context,
      const ::order_manager_proto::ManagerStatus* status,
      ::order_manager_proto::Reply* response) override;

  void RecordTracker(int& time_interval_in_minute);
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
  int PrintRecordResult();

  std::atomic<int64_t> order_id_;
  mutable std::mutex mutex_;
  std::unordered_map<int64_t, OrderStatus> order_id_to_order_status_;

  std::shared_ptr<OrderStore> order_store_;

  std::shared_ptr<MatchEngineStub> match_engine_stub_;

  std::atomic_bool record_is_start_ = false;
  std::atomic_int send_data_amount_ = 0;
  std::atomic_int receive_data_amount_ = 0;
  std::atomic_int latency_sum_ = 0;
  std::atomic_int latency_max_ = 0;
  std::atomic_int latency_min_ = 100000;
//  std::chrono::milliseconds latency_sum_ = std::chrono::milliseconds(0);
//  std::chrono::milliseconds latency_max_ = std::chrono::milliseconds(0);
//  std::chrono::milliseconds latency_min_ = std::chrono::milliseconds(100000000);
  mutable std::shared_mutex latency_mutex_;
  std::vector<int> send_data_list_;
  std::vector<int> receive_data_list_;
  //  std::chrono::time_point;
};

#endif  // ORDER_MANAGER_INCLUDE_ORDER_MANAGER_H_
