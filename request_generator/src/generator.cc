/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "src/generator.h"

#include <grpcpp/create_channel.h>

#include <memory>
#include <string>

#include "../../common/include/influxdb.hpp"
#include "../../common/include/json.hpp"
#include "protobuf_gen/order.grpc.pb.h"

bool Generator::PrepareOrders() {
  using json = nlohmann::json;
  std::string resp;
  influxdb_cpp::server_info si("127.0.0.1", 8086, "orders", "", "");
  int ret = influxdb_cpp::query(resp, "select  * from orders", si);
  if (0 == ret) {
    std::cout << "[INFO] query db success" << std::endl;
  } else {
    std::cout << "[ERROR] query db failed ret:" << ret << std::endl;
  }
  json j = json::parse(resp);

  int count = 0;
  for (auto& item : j["results"][0]["series"][0]["values"]) {
    order_manager_proto::Order order;
    order.set_amount(item[1]);
    order.set_price(item[2]);
    order.set_symbol(item[3]);
    order.set_trading_side(item[4]);
    order.set_user_id(item[5]);
    orders_.push(order);
    count++;
    if (count == num_of_orders_) break;
    //
    //    grpc::ClientContext query_context;
    //
    //    order_manager_proto::Reply reply;
    //    grpc::Status status = client.PlaceOrder(&query_context, order,
    //    &reply); std::cout << "#" << count << " reply message: " <<
    //    reply.message() << std::endl; std::cout << "#" << count << "reply
    //    error_code: " << reply.error_code() << std::endl;
  }
  if (count != num_of_orders_) {
    std::cout << "[WARNING] DB only has " << count << " orders" << std::endl;
    return false;
  } else {
    std::cout << "[INFO] successfully insert " << count << " orders"
              << std::endl;
    return true;
  }
}

void Generator::Start() {
  if (PrepareOrders() == false) {
    return;
  }

  for (int i = 0; i < num_of_threads_; i++) {
    std::queue<order_manager_proto::Order> orders;
    orders_for_thread_.push_back(orders);
  }

  int order_size = orders_.size();
  for (int i = 0; i < order_size; i++) {
    orders_for_thread_[i % num_of_threads_].push(orders_.front());
    orders_.pop();
  }

  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());
  for (int i = 0; i < num_of_threads_; i++) {
    std::cout << "[INFO] thread #" << i << " will handle "
              << orders_for_thread_[i].size() << " orders" << std::endl;
    std::thread th(SendRequest, orders_for_thread_[i], channel);
    th.join();
  }
}

void Generator::SendRequest(std::queue<order_manager_proto::Order> orders,
                            std::shared_ptr<grpc_impl::Channel> channel) {
  order_manager_proto::OrderManager::Stub clinet(channel);
  int count = 0;
  while (!orders.empty()) {
    grpc::ClientContext query_context;
    order_manager_proto::Reply reply;
    grpc::Status status =
        clinet.PlaceOrder(&query_context, orders.front(), &reply);
    //    std::cout << "#" << count << " reply message: " << reply.message() <<
    //    std::endl; std::cout << "#" << count << "reply error_code: " <<
    //    reply.error_code() << std::endl;
    orders.pop();
    count++;
  }
  mutex_requests_count_.lock();
  requests_count_ += count;
  mutex_requests_count_.unlock();
}

int Generator::getRequestsCount() { return requests_count_; }
