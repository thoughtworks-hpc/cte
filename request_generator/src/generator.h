/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_SRC_GENERATOR_H_
#define REQUEST_GENERATOR_SRC_GENERATOR_H_
#include <grpcpp/channel_impl.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../protobuf_gen/order.pb.h"
struct ip_address {
  ip_address(std::string ip, std::string port) : ip_(ip), port_(port) {}
  std::string ip_;
  std::string port_;
};

class Generator {
 private:
  int num_of_threads_;
  int num_of_orders_;
  static int requests_count_;

 public:
  static const std::vector<int>& getCountEachServer();

 private:
  static std::vector<int> count_each_server_;
  std::vector<ip_address> grpc_servers_;
  std::string db_host_address_;
  std::string db_port_;
  std::queue<order_manager_proto::Order> orders_;
  std::vector<std::queue<order_manager_proto::Order>> orders_for_thread_;
  static std::mutex mutex_requests_count_;

  bool PrepareOrders();
  static void SendRequest(std::queue<order_manager_proto::Order> orders,
                          std::vector<ip_address> grpc_servers);

 public:
  Generator(int num_of_threads, int num_of_orders,
            std::vector<ip_address>& grpc_server, std::string db_host_address,
            std::string db_port)
      : num_of_threads_(num_of_threads),
        num_of_orders_(num_of_orders),
        grpc_servers_(grpc_server),
        db_host_address_(std::move(db_host_address)),
        db_port_(std::move(db_port)) {}

  void Start();
  static int GetRequestsCount();
};

#endif  // REQUEST_GENERATOR_SRC_GENERATOR_H_
