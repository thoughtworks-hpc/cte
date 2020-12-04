/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_manager.h"

#include <cdcf/logger.h>

#include <utility>

#include "../../common/include/influxdb.hpp"
#include "../include/order_store_influxdb.h"

OrderManagerService::OrderManagerService(
    std::shared_ptr<OrderStore> order_store,
    std::shared_ptr<MatchEngineStub> match_engine_stub)
    : order_id_(0),
      order_store_(std::move(order_store)),
      match_engine_stub_(std::move(match_engine_stub)) {
  if (match_engine_stub_) {
    result_subscribe_thread_ = match_engine_stub_->SubscribeMatchResult(
        [this](const ::match_engine_proto::Trade &trade) {
          HandleMatchResult(trade);
        });
  }
}

OrderManagerService::~OrderManagerService() {
  if (result_subscribe_thread_ != nullptr) {
    result_subscribe_thread_->join();
  }
}

void OrderManagerService::RecordTracker(int &time_interval_in_seconds) {
  std::thread t([this, time_interval_in_seconds] {
    auto start_time = std::chrono::system_clock::now();
    send_data_list_.push_back(0);
    receive_data_list_.push_back(0);
    latency_list_.push_back(0);

    auto time_interval = std::chrono::seconds(time_interval_in_seconds);
    int send_data_amount_before = 0;
    int receive_data_amount_before = 0;
    int latency_sum_before = 0;
    while (record_is_start_) {
      auto time_now = std::chrono::system_clock::now();
      if (time_now - start_time > time_interval) {
        CDCF_LOGGER_INFO("In last {} seconds:", time_interval_in_seconds);
        auto send_data_now = send_data_amount_ - send_data_amount_before;
        auto receive_data_now =
            receive_data_amount_ - receive_data_amount_before;
        auto latency_sum_now = latency_sum_ - latency_sum_before;

        send_data_list_.push_back(send_data_now);
        receive_data_list_.push_back(receive_data_now);
        if (send_data_now != 0) {
          latency_list_.push_back(latency_sum_now / send_data_now);
        } else {
          latency_list_.push_back(0);
        }

        send_data_amount_before += send_data_now;
        receive_data_amount_before += receive_data_now;
        latency_sum_before += latency_sum_now;

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
    if (!record_is_start_) {
      CDCF_LOGGER_INFO("Order manger record is open.");
      record_is_start_ = true;
      RecordTracker(record_time_interval_);
    } else {
      CDCF_LOGGER_INFO("Order manger record is already open.");
    }
  } else if (::order_manager_proto::MANAGER_END == status->status()) {
    if (record_is_start_) {
      CDCF_LOGGER_INFO("Order manger record is close.");
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
  outfile << "Elapsed time(seconds),send amount,receive amount,latency average"
          << std::endl;
  for (int i = 0; i < send_data_list_.size(); i++) {
    outfile << i * record_time_interval_ << ",";
    outfile << send_data_list_[i] << ",";
    outfile << receive_data_list_[i] << ",";
    outfile << latency_list_[i] << "," << std::endl;
  }

  return 0;
}

::grpc::Status OrderManagerService::PlaceOrder(
    ::grpc::ServerContext *context, const ::order_manager_proto::Order *request,
    ::order_manager_proto::Reply *response) {
  Order order;
  match_engine_proto::Order match_engine_order;
  match_engine_proto::Reply reply;

  BuildOrder(*request, order, match_engine_order);

  SaveOrderStatus(order);
  if (!test_mode_is_open_) {
    order_store_->PersistOrder(order, "unsubmitted", 0);
  }

  std::string message;
  if (match_engine_stub_) {
    std::chrono::system_clock::time_point send_time =
        std::chrono::system_clock::now();
    if (record_is_start_) {
      send_data_amount_ += 1;
      send_time = std::chrono::system_clock::now();
    }

    ::grpc::Status status =
        match_engine_stub_->Match(match_engine_order, &reply);

    if (record_is_start_) {
      auto receive_time = std::chrono::system_clock::now();
      auto latency = std::chrono::duration_cast<std::chrono::milliseconds>(
          receive_time - send_time);
      latency_sum_ += latency.count();
      if (latency.count() < latency_min_ && latency.count() >= 0) {
        latency_min_ = latency.count();
      }
      if (latency.count() > latency_max_) {
        latency_max_ = latency.count();
      }
    }
    if (test_mode_is_open_) {
      return grpc::Status::OK;
    }

    int ret;
    if (reply.status() != match_engine_proto::STATUS_SUCCESS || !status.ok()) {
      ret = order_store_->PersistOrder(order, "submission error", 0);
    } else {
      ret = order_store_->PersistOrder(order, "submitted", 0);
    }
    if (0 == ret) {
      CDCF_LOGGER_DEBUG("submitted and saved order {}", order.order_id);
      message = "order submitted";
      response->set_error_code(order_manager_proto::SUCCESS);
    } else {
      CDCF_LOGGER_INFO("submission error for order {}", order.order_id);
      message = "order submission error";
      response->set_error_code(order_manager_proto::FAILURE);
    }
    response->set_message(message);
  }

  return grpc::Status::OK;
}

void OrderManagerService::SaveOrderStatus(const Order &order) {
  OrderStatus order_status{};

  order_status.order = order;
  order_status.concluded_amount = 0;
  std::lock_guard<std::mutex> lock(mutex_);
  order_id_to_order_status_[order.order_id] = order_status;
}

void OrderManagerService::HandleMatchResult(
    const ::match_engine_proto::Trade &trade) {
  if (record_is_start_) {
    receive_data_amount_ += 1;
  }

  int32_t maker_concluded_amount = 0;
  int32_t taker_concluded_amount = 0;
  Order maker_order;
  Order taker_order;
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

      if (maker_order.amount <= maker_concluded_amount) {
        if_maker_concluded = true;
        order_id_to_order_status_.erase(trade.maker_id());
      }
      if (taker_order.amount <= taker_concluded_amount) {
        if_taker_concluded = true;
        order_id_to_order_status_.erase(trade.taker_id());
      }
    } else {
      if_order_exists = false;
    }
  }

  if (test_mode_is_open_) {
    return;
  }

  if (if_order_exists) {
    CDCF_LOGGER_DEBUG(
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

void OrderManagerService::BuildOrder(
    const order_manager_proto::Order &request, Order &order,
    match_engine_proto::Order &match_engine_order) {
  using time_stamp = std::chrono::time_point<std::chrono::system_clock,
                                             std::chrono::nanoseconds>;
  time_stamp current_time_stamp =
      std::chrono::time_point_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now());
  int64_t nanoseconds_since_epoch =
      current_time_stamp.time_since_epoch().count();

  auto order_id = ++order_id_;

  order.order_id = order_id;
  order.amount = request.amount();
  order.user_id = request.user_id();
  order.symbol_id = request.symbol();
  order.price = request.price();
  order.submit_time = nanoseconds_since_epoch;

  if (request.trading_side() == order_manager_proto::TRADING_BUY) {
    order.trading_side = OrderTradingSide::Buy;
  } else if (request.trading_side() == order_manager_proto::TRADING_SELL) {
    order.trading_side = OrderTradingSide::Sell;
  } else {
    order.trading_side = OrderTradingSide::Unknown;
  }

  match_engine_order.set_order_id(order_id);
  match_engine_order.set_symbol_id(request.symbol());
  match_engine_order.set_user_id(request.user_id());
  match_engine_order.set_price(request.price());
  match_engine_order.set_amount(request.amount());
  match_engine_order.set_trading_side(
      static_cast<match_engine_proto::TradingSide>(
          static_cast<int>(request.trading_side())));
  match_engine_order.set_submit_time(nanoseconds_since_epoch);
}

void OrderManagerService::SetRecordTimeInterval(int interval) {
  record_time_interval_ = interval;
}

void OrderManagerService::SetLatencyAverageWarning(
    int latency_average_warning) {
  latency_average_warning_ = latency_average_warning;
}
void OrderManagerService::SetTestModeIsOpen(bool test_mode_is_open) {
  test_mode_is_open_ = test_mode_is_open;
}

::grpc::Status OrderManagerService::GetRunningStatus(
    ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
    ::order_manager_proto::RunningStatus *response) {
  int64_t order_status_map_size = 0;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    order_status_map_size = order_id_to_order_status_.size();
  }

  response->set_order_status_map_size(order_status_map_size);

  return grpc::Status::OK;
}

::grpc::Status OrderManagerService::GetOrderManagerDBCount(
    ::grpc::ServerContext *context, const ::google::protobuf::Empty *request,
    ::order_manager_proto::RunningStatus *response) {
  int64_t order_status_map_size = 0;

  {
    order_status_map_size =
        std::dynamic_pointer_cast<OrderStoreInfluxDB>(order_store_)
            ->GetDbCount();
  }

  response->set_order_status_map_size(order_status_map_size);
  return grpc::Status::OK;
}