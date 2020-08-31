/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "./order_store.h"

#ifndef CTE_ORDER_STORE_MOCK_H
#define CTE_ORDER_STORE_MOCK_H

class OrderStoreMock : public OrderStore {
 public:
  int PersistOrder(const match_engine_proto::Order& order, std::string status);
  int GetOrderByIndex(int index, match_engine_proto::Order& order,
                      std::string& status);

 private:
  std::vector<std::pair<match_engine_proto::Order, std::string> >
      order_and_status;
};

#endif  // CTE_ORDER_STORE_MOCK_H
