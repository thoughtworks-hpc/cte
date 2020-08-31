/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_manager.h"

#include <thread>
#include <utility>

#include "../../common/include/influxdb.hpp"

OrderManagerImpl::OrderManagerImpl(
    std::shared_ptr<OrderStore> order_store,
    const std::shared_ptr<Channel> &main_channel,
    const std::vector<const std::shared_ptr<Channel>> &request_channel)
    : order_id_(0),
      request_stub_index_(0),
      order_store_(std::move(order_store)) {
  if (main_channel != nullptr) {
    main_stub_ = ::match_engine_proto::TradingEngine::NewStub(main_channel);
    request_stubs_.push_back(main_stub_);
  }

  for (const auto &channel : request_channel) {
    request_stubs_.emplace_back(
        ::match_engine_proto::TradingEngine::NewStub(channel));
  }

  std::thread t(&OrderManagerImpl::SubscribeMatchResult, this);
  t.detach();
}

std::shared_ptr<TradingEngine::Stub> OrderManagerImpl::GetNextRequestStub() {
  assert(!request_stubs_.empty());

  if (request_stub_index_ != 0 &&
      request_stub_index_ == request_stubs_.size()) {
    request_stub_index_ = 0;
  }

  return request_stubs_[request_stub_index_++];
}

::grpc::Status OrderManagerImpl::PlaceOrder(
    ::grpc::ServerContext *context, const ::order_manager_proto::Order *request,
    ::order_manager_proto::Reply *response) {
  match_engine_proto::Order order;
  match_engine_proto::Reply reply;

  BuildMatchEngineOrder(*request, order);

  SaveOrderStatus(order);
  PersistOrder(order, "unsubmitted");

  ClientContext client_context;
  int ret = 0;
  if (!request_stubs_.empty()) {
    GetNextRequestStub()->Match(&client_context, order, &reply);

    if (reply.status() == match_engine_proto::STATUS_SUCCESS) {
      ret = PersistOrder(order, "order submitted");
    } else {
      ret = PersistOrder(order, "order submission error");
    }
  }

  std::string message;
  if (0 == ret) {
    std::cout << "submitted and saved order " << order.order_id() << std::endl;
    message = "order submitted";
    response->set_error_code(order_manager_proto::SUCCESS);
  } else {
    std::cout << "submission error for order " << order.order_id() << std::endl;
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

  if (main_stub_ != nullptr) {
    std::this_thread::sleep_for(std::chrono::seconds(10));

    std::unique_ptr<ClientReader<match_engine_proto::Trade>> reader(
        main_stub_->SubscribeMatchResult(&context, google::protobuf::Empty()));
    while (reader->Read(&trade)) {
      // trade.PrintDebugString();
      std::this_thread::sleep_for(std::chrono::seconds(2));
      {
        std::lock_guard<std::mutex> lock(mutex_);

        if (order_id_to_order_status_.find(trade.maker_id()) !=
                order_id_to_order_status_.end() &&
            order_id_to_order_status_.find(trade.taker_id()) !=
                order_id_to_order_status_.end()) {
          if_order_exists = true;
          auto &maker_order_status =
              order_id_to_order_status_[trade.maker_id()];
          auto &taker_order_status =
              order_id_to_order_status_[trade.taker_id()];
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
        std::cout << "receive trade for order " << trade.maker_id()
                  << " as maker"
                  << " and order " << trade.taker_id() << " as taker"
                  << std::endl;
        PersistOrder(maker_order, "transaction amount " +
                                      std::to_string(maker_transaction_amount));
        PersistOrder(taker_order, "transaction amount " +
                                      std::to_string(taker_transaction_amount));
      } else {
        std::cout << "order in trade doesn't exist for either "
                  << trade.maker_id() << " or " << trade.taker_id()
                  << std::endl;
      }
    }
    Status status = reader->Finish();
  }
}

void OrderManagerImpl::BuildMatchEngineOrder(
    const order_manager_proto::Order &request,
    match_engine_proto::Order &order) {
  using time_stamp = std::chrono::time_point<std::chrono::system_clock,
                                             std::chrono::nanoseconds>;
  time_stamp current_time_stamp =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now());
  int64_t nanoseconds_since_epoch =
      current_time_stamp.time_since_epoch().count();

  order.set_order_id(++order_id_);
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
  return order_store_->PersistOrder(order, status);

  //  influxdb_cpp::server_info si("127.0.0.1", 8086, "order_manager", "", "");
  //  std::string resp;
  //  std::string trading_side =
  //      order.trading_side() == match_engine_proto::TRADING_BUY ? "buy" :
  //      "sell";
  //
  //  int ret = influxdb_cpp::builder()
  //                .meas("order")
  //                .tag("order_id", std::to_string(order.order_id()))
  //                .tag("symbol_id", std::to_string(order.symbol()))
  //                .field("user_id", order.user_id())
  //                .field("price", order.price())
  //                .field("amount", order.amount())
  //                .field("trading_side", trading_side)
  //                .field("status", std::string(status))
  //                .timestamp(order.submit_time().seconds() * 1000000000 +
  //                           order.submit_time().nanos())
  //                .post_http(si, &resp);
  //
  //  if (0 == ret && resp.empty()) {
  //    std::cout << "write db success" << std::endl;
  //  } else {
  //    std::cout << "write db failed, ret:" << ret << " resp:" << resp
  //              << std::endl;
  //  }
  //
  //  return ret;
}
