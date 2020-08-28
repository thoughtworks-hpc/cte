/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_manager.h"

#include <thread>

#include "../../common/include/influxdb.hpp"

OrderManagerImpl::OrderManagerImpl(const std::shared_ptr<Channel> &channel)
    : order_id_(0),
      stub_(::match_engine_proto::TradingEngine::NewStub(channel)) {
  std::thread t(&OrderManagerImpl::SubscribeMatchResult, this);
  t.detach();
}

::grpc::Status OrderManagerImpl::PlaceOrder(
    ::grpc::ServerContext *context, const ::order_manager_proto::Order *request,
    ::order_manager_proto::Reply *response) {
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
  match_engine_proto::Order order;
  match_engine_proto::Reply reply;

  BuildMatchEngineOrder(*request, order_id, nanoseconds_since_epoch, order);

  SaveOrderStatus(order);
  //  int ret = influxdb_cpp::builder()
  //                .meas("order")
  //                .tag("order_id", std::to_string(order_id))
  //                .field("user_id", request->user_id())
  //                .field("price", request->price())
  //                .field("amount", request->amount())
  //                .field("trading_side", request->trading_side())
  //                .field("status", std::string("unsubmitted"))
  //                .timestamp(nanoseconds_since_epoch)
  //                .post_http(si, &resp);
  PersistOrder(order, "unsubmitted");

  ClientContext client_context;

  //  order.set_order_id(order_id);
  //  order.set_symbol(request->symbol());
  //  order.set_user_id(request->user_id());
  //  order.set_price(request->price());
  //  order.set_amount(request->amount());
  //  order.set_trading_side(static_cast<match_engine_proto::TradingSide>(
  //      static_cast<int>(request->trading_side())));
  //  auto submit_time = new google::protobuf::Timestamp{};
  //  submit_time->set_seconds(nanoseconds_since_epoch / 1000000000);
  //  submit_time->set_nanos(nanoseconds_since_epoch % 1000000000);
  //  order.set_allocated_submit_time(submit_time);

  Status status = stub_->Match(&client_context, order, &reply);
  int ret;
  if (reply.status() == match_engine_proto::STATUS_SUCCESS) {
    //    influxdb_cpp::builder()
    //        .meas("order")
    //        .tag("order_id", std::to_string(order_id))
    //        .field("user_id", request->user_id())
    //        .field("price", request->price())
    //        .field("amount", request->amount())
    //        .field("trading_side", request->trading_side())
    //        .field("status", std::string("submitted"))
    //        .timestamp(nanoseconds_since_epoch)
    //        .post_http(si, &resp);
    ret = PersistOrder(order, "submitted");
  } else {
    //    influxdb_cpp::builder()
    //        .meas("order")
    //        .tag("order_id", std::to_string(order_id))
    //        .field("user_id", request->user_id())
    //        .field("price", request->price())
    //        .field("amount", request->amount())
    //        .field("trading_side", request->trading_side())
    //        .field("status", std::string("submission error"))
    //        .timestamp(nanoseconds_since_epoch)
    //        .post_http(si, &resp);
    ret = PersistOrder(order, "submission error");
  }

  std::string message;
  if (0 == ret) {
    message = "order submitted";
    response->set_error_code(order_manager_proto::SUCCESS);
  } else {
    message = "order submission error";
    response->set_error_code(order_manager_proto::FAILURE);
  }
  response->set_message(message);

  return grpc::Status::OK;
}

void OrderManagerImpl::SaveOrderStatus(const match_engine_proto::Order &order) {
  OrderStatus order_status;
  order_status.order = order;
  order_status.transaction_amount = 0;
  std::cout << "save order status for order " << order.order_id() << std::endl;
  std::lock_guard<std::mutex> lock(mutex_);
  order_id_to_order_status_[order.order_id()] = order_status;
}

void OrderManagerImpl::SubscribeMatchResult() {
  ClientContext context;
  match_engine_proto::Trade trade;
  int32_t maker_transaction_amount = 0;
  int32_t taker_transaction_amount = 0;
  match_engine_proto::Order maker_order;
  match_engine_proto::Order taker_order;
  bool if_order_exists = false;
  std::this_thread::sleep_for(std::chrono::seconds(10));

  std::unique_ptr<ClientReader<match_engine_proto::Trade> > reader(
      stub_->SubscribeMatchResult(&context, google::protobuf::Empty()));
  while (reader->Read(&trade)) {
    trade.PrintDebugString();
    std::cout << "trade: " << trade.SerializeAsString() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    {
      std::lock_guard<std::mutex> lock(mutex_);

      if (order_id_to_order_status_.find(trade.maker_id()) !=
              order_id_to_order_status_.end() &&
          order_id_to_order_status_.find(trade.taker_id()) !=
              order_id_to_order_status_.end()) {
        if_order_exists = true;
        auto &maker_order_status = order_id_to_order_status_[trade.maker_id()];
        auto &taker_order_status = order_id_to_order_status_[trade.taker_id()];
        maker_order_status.transaction_amount =
            maker_order_status.transaction_amount + trade.amount();
        taker_order_status.transaction_amount =
            taker_order_status.transaction_amount + trade.amount();
        maker_order = maker_order_status.order;
        taker_order = taker_order_status.order;
        maker_transaction_amount = maker_order_status.transaction_amount;
        taker_transaction_amount = taker_order_status.transaction_amount;
      } else {
        if_order_exists = false;
      }
    }

    if (if_order_exists) {
      std::cout << "received order transaction for order " << trade.maker_id()
                << " as maker" << std::endl;
      std::cout << "received order transaction for order " << trade.taker_id()
                << " as taker" << std::endl;
      PersistOrder(maker_order, "transaction amount " +
                                    std::to_string(maker_transaction_amount));
      PersistOrder(taker_order, "transaction amount " +
                                    std::to_string(taker_transaction_amount));
    } else {
      std::cout << "order in trade doesn't exist for either "
                << trade.maker_id() << " or " << trade.taker_id() << std::endl;
    }
  }
  Status status = reader->Finish();
}

void OrderManagerImpl::BuildMatchEngineOrder(
    const order_manager_proto::Order &request, int64_t order_id,
    int64_t nanoseconds_since_epoch, match_engine_proto::Order &order) {
  order.set_order_id(order_id);
  order.set_symbol(request.symbol());
  order.set_user_id(request.user_id());
  order.set_price(request.price());
  order.set_amount(request.amount());
  order.set_trading_side(static_cast<match_engine_proto::TradingSide>(
      static_cast<int>(request.trading_side())));
  auto submit_time = new google::protobuf::Timestamp{};
  submit_time->set_seconds(nanoseconds_since_epoch / 1000000000);
  submit_time->set_nanos(nanoseconds_since_epoch % 1000000000);
  order.set_allocated_submit_time(submit_time);
}

int OrderManagerImpl::PersistOrder(const match_engine_proto::Order &order,
                                   std::string status) {
  influxdb_cpp::server_info si("127.0.0.1", 8086, "order_manager", "", "");
  std::string resp;

  int ret = influxdb_cpp::builder()
                .meas("order")
                .tag("order_id", std::to_string(order.order_id()))
                .field("user_id", order.user_id())
                .field("price", order.price())
                .field("amount", order.amount())
                .field("trading_side", order.trading_side())
                .field("status", std::string(status))
                .timestamp(order.submit_time().seconds() * 1000000000 +
                           order.submit_time().nanos())
                .post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    std::cout << "write db success" << std::endl;
  } else {
    std::cout << "write db failed, ret:" << ret << " resp:" << resp
              << std::endl;
  }

  return ret;
}
