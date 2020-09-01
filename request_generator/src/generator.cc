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

int Generator::requests_count_ = 0;
std::mutex Generator::mutex_requests_count_;
std::vector<int> Generator::count_each_server_;

bool Generator::PrepareOrders() {
  using json = nlohmann::json;
  std::string resp;
  influxdb_cpp::server_info si(db_host_address_, std::stoi(db_port_), "orders",
                               "", "");
  int ret = influxdb_cpp::query(resp, "select  * from orders", si);
  if (0 == ret) {
    std::cout << "[INFO] query db success" << std::endl;
  } else {
    std::cout << "[ERROR] query db failed ret:" << ret << std::endl;
    return false;
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
  }
  if (count < num_of_orders_) {
    std::cout << "[WARNING] DB only has " << count << " orders" << std::endl;
    std::cout << "[INFO] insert " << count << " orders" << std::endl;
  } else if (num_of_orders_ == -1) {
    std::cout << "[INFO] Insert all " << count << " orders in DB" << std::endl;
  } else {
    std::cout << "[INFO] successfully insert " << count << " orders"
              << std::endl;
  }
  return true;
}

void Generator::Start() {
  if (!PrepareOrders()) {
    return;
  }
  // auto last_check_order = orders_.front();
  for (int i = 0; i < num_of_threads_; i++) {
    std::queue<order_manager_proto::Order> orders;
    orders_for_thread_.push_back(orders);
  }

  int order_size = orders_.size();
  for (int i = 0; i < order_size; i++) {
    orders_for_thread_[i % num_of_threads_].push(orders_.front());
    orders_.pop();
  }

  std::vector<std::thread*> thread_vector;

  for (int i = 0; i < num_of_threads_; i++) {
    std::thread* th =
        new std::thread(SendRequest, orders_for_thread_[i], grpc_servers_);
    thread_vector.emplace_back(th);
    std::cout << "[INFO] thread #" << th->get_id() << " starts" << std::endl;
  }

  for (auto t : thread_vector) {
    t->join();
  }
}

void Generator::SendRequest(std::queue<order_manager_proto::Order> orders,
                            std::vector<ip_address> grpc_servers) {
  std::vector<std::shared_ptr<grpc_impl::Channel>> channels;
  std::vector<order_manager_proto::OrderManager::Stub> clients;
  for (auto server : grpc_servers) {
    // std::cout << server.ip_ << ":" << server.port_ << std::endl;
    auto channel = grpc::CreateChannel(server.ip_ + ":" + server.port_,
                                       grpc::InsecureChannelCredentials());
    channels.emplace_back(channel);
    order_manager_proto::OrderManager::Stub clinet(channel);
    clients.emplace_back(clinet);
  }

  int count = 0;
  int server_index = 0;
  std::vector<int> count_each_server(grpc_servers.size(), 0);
  while (!orders.empty()) {
    grpc::ClientContext query_context;
    order_manager_proto::Reply reply;
    grpc::Status status =
        clients[server_index % grpc_servers.size()].PlaceOrder(
            &query_context, orders.front(), &reply);
    orders.pop();

    if (status.ok() &&
        reply.error_code() == order_manager_proto::ErrorCode::FAILURE) {
      count_each_server[server_index % grpc_servers.size()]++;
      count++;
    } else {
      std::cout << "[ERROR] request "
                << grpc_servers[server_index % grpc_servers.size()].ip_ << ":"
                << grpc_servers[server_index % grpc_servers.size()].port_
                << " failed" << std::endl;
    }
    server_index++;
  }
  mutex_requests_count_.lock();
  requests_count_ += count;
  if (count_each_server_.empty()) {
    for (auto count : count_each_server) {
      count_each_server_.push_back(count);
    }
  } else {
    for (int i = 0; i < count_each_server.size(); i++) {
      count_each_server_[i] += count_each_server[i];
    }
  }
  mutex_requests_count_.unlock();

  std::cout << "[INFO] thread #" << std::this_thread::get_id() << " finished"
            << std::endl;
  //  for (int i = 0; i < count_each_server.size(); i++) {
  //    std::cout << "[INFO] thread #" << std::this_thread::get_id() << " send
  //    #"
  //              << i << " : " << count_each_server[i] << std::endl;
  //  }
}

int Generator::GetRequestsCount() { return requests_count_; }
const std::vector<int>& Generator::getCountEachServer() {
  return count_each_server_;
}
