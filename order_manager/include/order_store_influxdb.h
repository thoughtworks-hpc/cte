/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_ORDER_STORE_INFLUXDB_H_
#define ORDER_MANAGER_INCLUDE_ORDER_STORE_INFLUXDB_H_

#include <string>

#include "./order_store.h"

class OrderStoreInfluxDB : public OrderStore {
 public:
  OrderStoreInfluxDB(const std::string& host, int port)
      : host_(host), port_(port) {}
  int PersistOrder(const match_engine_proto::Order& order, std::string status,
                   int concluded_amount) override;

 private:
  std::string host_;
  int port_;
};

#endif  // ORDER_MANAGER_INCLUDE_ORDER_STORE_INFLUXDB_H_
