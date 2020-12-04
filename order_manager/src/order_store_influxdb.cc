/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_store_influxdb.h"

#include <cdcf/logger.h>

#include "../../common/include/influxdb.hpp"

OrderStoreInfluxDB::OrderStoreInfluxDB(const DatabaseConfig& config,
                                       database_interface::InfluxDB* influxdb)
    : host_(config.db_address), port_(config.db_port), influxdb_(influxdb) {
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

int OrderStoreInfluxDB::PersistOrder(const Order& order, std::string status,
                                     int concluded_amount) {
  influxdb_cpp::server_info si(host_, port_, database_, user_, password_);
  std::string resp;

  std::string trading_side;
  if (order.trading_side == OrderTradingSide::Buy) {
    trading_side = "buy";
  } else if (order.trading_side == OrderTradingSide::Sell) {
    trading_side = "sell";
  } else {
    trading_side = "unknown";
  }

  std::vector<database_interface::data_pair> tag;
  std::vector<database_interface::data_pair> field;
  field.emplace_back(database_interface::data_pair{
      "order_id", std::to_string(order.order_id)});
  field.emplace_back(database_interface::data_pair{
      "symbol_id", std::to_string(order.symbol_id)});
  field.emplace_back(
      database_interface::data_pair{"user_id", std::to_string(order.user_id)});
  field.emplace_back(
      database_interface::data_pair{"price", std::to_string(order.price)});
  field.emplace_back(
      database_interface::data_pair{"amount", std::to_string(order.amount)});
  field.emplace_back(
      database_interface::data_pair{"trading_side", trading_side});
  field.emplace_back(
      database_interface::data_pair{"status", std::string(status)});
  field.emplace_back(database_interface::data_pair{
      "concluded amount", std::to_string(concluded_amount)});

  database_interface::entity payload{"order", tag, field, order.submit_time};
  if (influxdb_->write(payload)) {
    return 0;
  } else {
    return -1;
  }
}

int OrderStoreInfluxDB::GetDbCount() { return influxdb_->get_count(); }
