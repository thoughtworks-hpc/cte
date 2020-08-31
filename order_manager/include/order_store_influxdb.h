/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef CTE_ORDER_STORE_IMPL_H
#define CTE_ORDER_STORE_IMPL_H

#include "./order_store.h"

class OrderStoreInfluxDB : public OrderStore {
 public:
  int PersistOrder(const match_engine_proto::Order &order,
                   std::string status) override;

 private:
  std::string host_;
  int port_;
};

#endif  // CTE_ORDER_STORE_IMPL_H
