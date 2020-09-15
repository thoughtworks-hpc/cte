/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_manager.h"

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

void OrderManagerService::RecordTracker(int &time_interval_in_seconds) {
  std::thread t([this, time_interval_in_seconds] {
    auto start_time = std::chrono::system_clock::now();
    send_data_list_.push_back(0);
    receive_data_list_.push_back(0);
    auto time_interval = std::chrono::seconds (time_interval_in_seconds);
    auto send_data_amount_before = 0;
    auto receive_data_amount_before = 0;
    while (record_is_start_) {
      auto time_now = std::chrono::system_clock::now();
      if (time_now - start_time > time_interval) {
        CDCF_LOGGER_INFO("In {} seconds:", time_interval_in_seconds);
        auto send_data_now = send_data_amount_ - send_data_amount_before;
        auto receive_data_now =
            receive_data_amount_ - receive_data_amount_before;
        send_data_amount_before += send_data_now;
        receive_data_amount_before += receive_data_now;
        send_data_list_.push_back(send_data_now);
        receive_data_list_.push_back(receive_data_now);
        CDCF_LOGGER_INFO("Send data amount: {}", send_data_now,
                         time_interval_in_seconds);
        CDCF_LOGGER_INFO("Receive data amount: {}", receive_data_now,
                         time_interval_in_seconds);
        if (send_data_amount_ != 0) {
          auto latency_average = latency_sum_ / send_data_amount_;
          CDCF_LOGGER_INFO("Latency average: {} milliseconds", latency_average);
          CDCF_LOGGER_INFO("Latency max: {} milliseconds", latency_max_);
          CDCF_LOGGER_INFO("Latency min: {} milliseconds", latency_min_);
          if (latency_average >= latency_average_warning_) {
            CDCF_LOGGER_WARN("Latency average is more than {} milliseconds",
                             latency_average_warning_);
          }
        }
        start_time = time_now;
      }
    }
  });
  t.detach();
}

::grpc::Status OrderManagerService::StartEndManager(
    ::grpc::ServerContext *context,
    const ::order_manager_proto::ManagerStatus *status,
    ::order_manager_proto::Reply *response) {
  if (::order_manager_proto::MANAGER_START == status->status()) {
    CDCF_LOGGER_INFO("Order manger record is open.");
    if (!record_is_start_) {
      record_is_start_ = true;
      RecordTracker(record_time_interval_);
    } else {
      CDCF_LOGGER_INFO("Order manger record is already open.");
    }
  } else if (::order_manager_proto::MANAGER_END == status->status()) {
    CDCF_LOGGER_INFO("Order manger record is close.");
    if (record_is_start_) {
      PrintRecordResult();
      record_is_start_ = false;
      send_data_amount_ = 0;
      receive_data_amount_ = 0;
      latency_sum_ = 0;
      latency_max_ = 0;
      latency_min_ = 100000;
      send_data_list_.clear();
      receive_data_list_.clear();
    } else {
      CDCF_LOGGER_INFO("Order manger record is already close.");
    }
  }

  response->set_error_code(order_manager_proto::SUCCESS);
  return grpc::Status::OK;
}

int OrderManagerService::PrintRecordResult() {
  if (!record_is_start_) {
    CDCF_LOGGER_INFO("Record is empty without start!");
    return 1;
  }
  std::ofstream outfile("performance_testing.csv");
  outfile << "This is the total performance:" << std::endl;
  outfile << "Manager send," << send_data_amount_ << std::endl;
  outfile << "Manager receive," << receive_data_amount_ << std::endl;
  if (send_data_amount_ != 0) {
    auto latency_average = latency_sum_ / send_data_amount_;
    outfile << "Latency average," << latency_average << " milliseconds"
            << std::endl;
    outfile << "Latency max," << latency_max_ << " milliseconds" << std::endl;
    outfile << "Latency min," << latency_min_ << " milliseconds" << std::endl;
  }
  outfile << " " << std::endl;
  outfile << "Elapsed time(seconds),send amount,receive amount" << std::endl;
  for (int i = 0; i < send_data_list_.size(); i++) {
    outfile << i * record_time_interval_ << ",";
    outfile << send_data_list_[i] << ",";
    outfile << receive_data_list_[i] << "," << std::endl;
  }

  return 0;
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
    std::chrono::system_clock::time_point send_time =
        std::chrono::system_clock::now();
    if (record_is_start_) {
      send_data_amount_ += 1;
      send_time = std::chrono::system_clock::now();
      std::cout << "send data amount: " << send_data_amount_ << std::endl;
    }

    match_engine_stub_->Match(order, &reply);

    if (record_is_start_) {
      auto receive_time = std::chrono::system_clock::now();
      auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
          receive_time - send_time);
      latency_sum_ += latency.count();
      if (latency.count() < latency_min_) {
        latency_min_ = latency.count();
      }
      if (latency.count() > latency_max_) {
        latency_max_ = latency.count();
      }
    }

    int ret;

    if (reply.status() == match_engine_proto::STATUS_SUCCESS) {
      ret = order_store_->PersistOrder(order, "submitted", 0);
    } else {
      ret = order_store_->PersistOrder(order, "submission error", 0);
    }
    if (0 == ret) {
      std::cout << "submitted and saved order " << order.order_id()
                << std::endl;
      message = "order submitted";
      response->set_error_code(order_manager_proto::SUCCESS);
    } else {
      std::cout << "submission error for order " << order.order_id()
                << std::endl;
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

  if (record_is_start_) {
    receive_data_amount_ += 1;
    std::cout << "receive data amount: " << receive_data_amount_ << std::endl;
  }

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
    std::cout << "receive trade for order " << trade.maker_id() << " as maker"
              << " and order " << trade.taker_id() << " as taker" << std::endl;
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
    std::cout << "order in trade doesn't exist for either " << trade.maker_id()
              << " or " << trade.taker_id() << std::endl;
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
void OrderManagerService::SetRecordTimeInterval(int interval) {
  record_time_interval_ = interval;
}
void OrderManagerService::SetLatencyAverageWarning(int latency_average_warning) {
  latency_average_warning_ = latency_average_warning;
}
