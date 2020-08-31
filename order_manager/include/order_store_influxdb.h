/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_ORDER_STORE_IMPL_H
#define CTE_ORDER_STORE_IMPL_H

#include "./order_store.h"

class OrderStoreInfluxDB : public OrderStore {
 public:
  OrderStoreInfluxDB(const std::string& host, int port)
      : host_(host), port_(port) {}
  int PersistOrder(const match_engine_proto::Order& order,
                   std::string status) override;

 private:
  std::string host_;
  int port_;
};

#endif  // CTE_ORDER_STORE_IMPL_H
