/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_INCLUDE_GENERATOR_H_
#define REQUEST_GENERATOR_INCLUDE_GENERATOR_H_
#include <grpcpp/channel_impl.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../../common/protobuf_gen/order_manager.grpc.pb.h"
#include "../../common/protobuf_gen/order_manager.pb.h"
#include "./database_interface.h"

struct ip_address {
  ip_address(std::string ip, std::string port)
      : ip_(std::move(ip)), port_(std::move(port)) {}
  std::string ip_;
  std::string port_;
};

class Generator {
 public:
  static int requests_count_;
  static std::mutex mutex_requests_count_;

  static std::vector<int> count_each_server_;

  Generator(int num_of_threads, int num_of_orders,
            std::vector<ip_address>& grpc_server,
            DatabaseQueryInterface* database)
      : num_of_threads_(num_of_threads),
        num_of_orders_(num_of_orders),
        grpc_servers_(grpc_server),
        database_(database) {
    for (int i = 0; i < num_of_threads_; i++) {
      std::queue<order_manager_proto::Order> orders;
      orders_for_thread_.push_back(orders);
    }
  }

  void Start();

 private:
  DatabaseQueryInterface* database_;
  int num_of_threads_;
  int num_of_orders_;
  std::vector<ip_address> grpc_servers_;
  std::queue<order_manager_proto::Order> orders_;
  std::vector<std::queue<order_manager_proto::Order>> orders_for_thread_;
  std::unordered_map<int, int> symbol_to_thread_;

  bool PrepareOrders();
  static void SendRequest(std::queue<order_manager_proto::Order> orders,
                          std::vector<ip_address> grpc_servers);
};

#endif  // REQUEST_GENERATOR_INCLUDE_GENERATOR_H_
