/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_ORDER_STORE_INFLUXDB_H_
#define ORDER_MANAGER_INCLUDE_ORDER_STORE_INFLUXDB_H_

#include <string>

#include "../../common/include/database_interface.hpp"
#include "./order_store.h"

struct DatabaseConfig {
  std::string db_address;
  int db_port;
  std::string db_user;
  std::string db_password;
  std::string db_name;
  std::string db_measurement;
};

class OrderStoreInfluxDB : public OrderStore {
 public:
  explicit OrderStoreInfluxDB(const DatabaseConfig& config,
                              database_interface::InfluxDB* influxdb);
  int PersistOrder(const Order& order, std::string status,
                   int concluded_amount) override;

  int GetDbCount();

 private:
  std::string host_;
  int port_;
  std::string database_ = "order_manager";
  std::string measurement_ = "order";
  std::string user_ = "order_manager";
  std::string password_ = "order";
  database_interface::InfluxDB* influxdb_;
};

#endif  // ORDER_MANAGER_INCLUDE_ORDER_STORE_INFLUXDB_H_
