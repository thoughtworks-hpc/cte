/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_manager.h"

#include <cdcf/logger.h>

#include <thread>
#include <utility>

#include "../../common/include/influxdb.hpp"

OrderManagerService::OrderManagerService(
    std::shared_ptr<OrderStore> order_store,
    std::shared_ptr<MatchEngineStub> match_engine_stub)
    : order_id_(0),
      order_store_(std::move(order_store)),
      match_engine_stub_(std::move(match_engine_stub)) {
  if (match_engine_stub_) {
    match_engine_stub_->SubscribeMatchResult(
        [this](const ::match_engine_proto::Trade &trade) {
          HandleMatchResult(trade);
        });
  }
}

::grpc::Status OrderManagerService::PlaceOrder(
    ::grpc::ServerContext *context, const ::order_manager_proto::Order *request,
    ::order_manager_proto::Reply *response) {
  match_engine_proto::Order order;
  match_engine_proto::Reply reply;

  BuildMatchEngineOrder(*request, order);

  SaveOrderStatus(order);
  order_store_->PersistOrder(order, "unsubmitted", 0);

  std::string message;
  if (match_engine_stub_) {
    match_engine_stub_->Match(order, &reply);
    int ret = 0;
    if (reply.status() == match_engine_proto::STATUS_SUCCESS) {
      ret = order_store_->PersistOrder(order, "submitted", 0);
    } else {
      ret = order_store_->PersistOrder(order, "submission error", 0);
    }
    if (0 == ret) {
      CDCF_LOGGER_INFO("submitted and saved order {}", order.order_id());
      message = "order submitted";
      response->set_error_code(order_manager_proto::SUCCESS);
    } else {
      CDCF_LOGGER_INFO("submission error for order {}", order.order_id());
      message = "order submission error";
      response->set_error_code(order_manager_proto::FAILURE);
    }
    response->set_message(message);
  }

  return grpc::Status::OK;
}

void OrderManagerService::SaveOrderStatus(
    const match_engine_proto::Order &order) {
  OrderStatus order_status;
  order_status.order = order;
  order_status.concluded_amount = 0;
  std::lock_guard<std::mutex> lock(mutex_);
  order_id_to_order_status_[order.order_id()] = order_status;
}

void OrderManagerService::HandleMatchResult(
    const ::match_engine_proto::Trade &trade) {
  int32_t maker_concluded_amount = 0;
  int32_t taker_concluded_amount = 0;
  match_engine_proto::Order maker_order;
  match_engine_proto::Order taker_order;
  bool if_order_exists = false;
  bool if_maker_concluded = false;
  bool if_taker_concluded = false;

  {
    std::lock_guard<std::mutex> lock(mutex_);

    if (order_id_to_order_status_.find(trade.maker_id()) !=
            order_id_to_order_status_.end() &&
        order_id_to_order_status_.find(trade.taker_id()) !=
            order_id_to_order_status_.end()) {
      if_order_exists = true;
      auto &maker_order_status = order_id_to_order_status_[trade.maker_id()];
      auto &taker_order_status = order_id_to_order_status_[trade.taker_id()];
      maker_order_status.concluded_amount =
          maker_order_status.concluded_amount + trade.amount();
      taker_order_status.concluded_amount =
          taker_order_status.concluded_amount + trade.amount();
      maker_order = maker_order_status.order;
      taker_order = taker_order_status.order;
      maker_concluded_amount = maker_order_status.concluded_amount;
      taker_concluded_amount = taker_order_status.concluded_amount;

      if (maker_order.amount() <= maker_concluded_amount) {
        if_maker_concluded = true;
      }
      if (taker_order.amount() <= taker_concluded_amount) {
        if_taker_concluded = true;
      }
    } else {
      if_order_exists = false;
    }
  }

  if (if_order_exists) {
    CDCF_LOGGER_INFO(
        "receive trade for order {} as maker and order {} as taker",
        trade.maker_id(), trade.taker_id());
    std::string maker_status;
    std::string taker_status;
    if (if_maker_concluded) {
      maker_status = "concluded";
    } else {
      maker_status = "partially concluded";
    }

    if (if_taker_concluded) {
      taker_status = "concluded";
    } else {
      taker_status = "partially concluded";
    }

    order_store_->PersistOrder(maker_order, maker_status,
                               maker_concluded_amount);
    order_store_->PersistOrder(taker_order, taker_status,
                               taker_concluded_amount);
  } else {
    CDCF_LOGGER_ERROR("order in trade doesn't exist for either {} or {}",
                      trade.maker_id(), trade.taker_id());
  }
}

void OrderManagerService::BuildMatchEngineOrder(
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
  order.set_symbol_id(request.symbol());
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
