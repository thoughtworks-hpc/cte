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

  influxdb_cpp::server_info si("127.0.0.1", 8086, "order_manager", "", "");
  std::string resp;

  int ret = influxdb_cpp::builder()
                .meas("order")
                .tag("order_id", std::to_string(++order_id_))
                .field("user_id", request->user_id())
                .field("price", request->price())
                .field("amount", request->amount())
                .field("trading_side", request->trading_side())
                .field("status", std::string("received"))
                .timestamp(current_time_stamp.time_since_epoch().count())
                .post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    std::cout << "write db success" << std::endl;
  } else {
    std::cout << "write db failed, ret:" << ret << " resp:" << resp
              << std::endl;
  }

  response->set_error(27);

  //  stub_->Match(&client_context, order, reply);
  return grpc::Status::OK;
}
