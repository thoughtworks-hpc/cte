/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef REQUEST_GENERATOR_SRC_GENERATOR_H_
#define REQUEST_GENERATOR_SRC_GENERATOR_H_
#include <grpcpp/channel_impl.h>

#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "../protobuf_gen/order.pb.h"

class Generator {
 private:
  int num_of_threads_;
  int num_of_orders_;
  static int requests_count_;
  std::queue<order_manager_proto::Order> orders_;
  std::vector<std::thread> threads_;
  std::vector<std::queue<order_manager_proto::Order>> orders_for_thread_;
  static std::mutex mutex_requests_count_;

  bool PrepareOrders();
  static void SendRequest(std::queue<order_manager_proto::Order> orders,
                          std::shared_ptr<grpc_impl::Channel> channel);

 public:
  explicit Generator(int num_of_threads)
      : num_of_threads_(num_of_threads), num_of_orders_(10000) {}

  Generator(int num_of_threads, int num_of_orders)
      : num_of_threads_(num_of_threads), num_of_orders_(num_of_orders) {}
  void Start();
  static int getRequestsCount();
};

#endif  // REQUEST_GENERATOR_SRC_GENERATOR_H_
