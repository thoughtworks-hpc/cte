/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_manager.h"

#include "../../common/include/influxdb.hpp"

::grpc::Status OrderManagerImpl::PlaceOrder(
    ::grpc::ServerContext *context, const ::order_manager_proto::Order *request,
    ::order_manager_proto::Reply *response) {
  ClientContext client_context;
  match_engine_proto::Order order;
  match_engine_proto::Reply *reply = nullptr;

  using time_stamp = std::chrono::time_point<std::chrono::system_clock,
                                             std::chrono::nanoseconds>;
  time_stamp current_time_stamp =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now());
  int64_t nanoseconds_since_epoch =
      current_time_stamp.time_since_epoch().count();

  influxdb_cpp::server_info si("127.0.0.1", 8086, "order_manager", "", "");
  std::string resp;

  int64_t order_id = ++order_id_;
  int ret = influxdb_cpp::builder()
                .meas("order")
                .tag("order_id", std::to_string(order_id))
                .field("user_id", request->user_id())
                .field("price", request->price())
                .field("amount", request->amount())
                .field("trading_side", request->trading_side())
                .field("status", std::string("received"))
                .timestamp(nanoseconds_since_epoch)
                .post_http(si, &resp);

  std::string message;
  if (0 == ret && resp.empty()) {
    message = "write db success";
    response->set_error_code(order_manager_proto::SUCCESS);
    //    std::cout << "write db success" << std::endl;
  } else {
    message = message + "write db failed, ret:" + std::to_string(ret) +
              " resp:" + resp;
    response->set_error_code(order_manager_proto::FAILURE);
    //    std::cout << "write db failed, ret:" << ret << " resp:" << resp
    //              << std::endl;
  }

  response->set_message(message);

  order.set_order_id(order_id);
  order.set_symbol(request->symbol());
  order.set_user_id(request->user_id());
  order.set_price(request->price());
  order.set_amount(request->amount());
  order.set_trading_side(static_cast<match_engine_proto::TradingSide>(
      static_cast<int>(request->trading_side())));
  google::protobuf::Timestamp submit_time;
  submit_time.set_nanos(nanoseconds_since_epoch);
  order.set_allocated_submit_time(&submit_time);

  //  stub_->Match(&client_context, order, reply);
  return grpc::Status::OK;
}
