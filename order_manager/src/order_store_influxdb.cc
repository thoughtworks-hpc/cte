/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_store_influxdb.h"

#include <cdcf/logger.h>

#include "../../common/include/influxdb.hpp"

OrderStoreInfluxDB::OrderStoreInfluxDB(const DatabaseConfig& config)
    : host_(config.db_address), port_(config.db_port) {
  std::string resp;
  int ret;

  if (host_ == "localhost") {
    host_ = "127.0.0.1";
  }

  if (!config.db_name.empty()) {
    database_ = config.db_name;
  }

  if (!config.db_measurement.empty()) {
    measurement_ = config.db_measurement;
  }

  if (!config.db_user.empty()) {
    user_ = config.db_user;
  }

  if (!config.db_password.empty()) {
    password_ = config.db_password;
  }

  influxdb_cpp::server_info si(host_, port_, "", user_, password_);
  ret = influxdb_cpp::create_db(resp, database_, si);
  if (0 != ret) {
    CDCF_LOGGER_ERROR("create database of {} failed ret: {}", database_, ret);
  }
}

int OrderStoreInfluxDB::PersistOrder(const match_engine_proto::Order& order,
                                     std::string status, int concluded_amount) {
  influxdb_cpp::server_info si(host_, port_, database_, user_, password_);
  std::string resp;

  std::string trading_side;
  if (order.trading_side() == match_engine_proto::TRADING_BUY) {
    trading_side = "buy";
  } else if (order.trading_side() == match_engine_proto::TRADING_SELL) {
    trading_side = "sell";
  } else {
    trading_side = "unknown";
  }

  int ret = influxdb_cpp::builder()
                .meas(measurement_)
                .tag("order_id", std::to_string(order.order_id()))
                .tag("symbol_id", std::to_string(order.symbol_id()))
                .field("user_id", order.user_id())
                .field("price", order.price())
                .field("amount", order.amount())
                .field("trading_side", trading_side)
                .field("status", std::string(status))
                .field("concluded amount", concluded_amount)
                .timestamp(order.submit_time())
                .post_http(si, &resp);

  if (0 == ret && resp.empty()) {
    CDCF_LOGGER_DEBUG("write db success");
  } else {
    CDCF_LOGGER_ERROR("write db failed, ret: {}", resp);
  }

  return ret;
}
