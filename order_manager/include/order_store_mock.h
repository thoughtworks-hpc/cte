/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_ORDER_STORE_MOCK_H_
#define ORDER_MANAGER_INCLUDE_ORDER_STORE_MOCK_H_

#include <string>
#include <utility>
#include <vector>

#include "./order_store.h"

class OrderStoreMock : public OrderStore {
 public:
  int PersistOrder(const match_engine_proto::Order& order, std::string status,
                   int concluded_amount);
  int GetOrderByIndex(int index, match_engine_proto::Order& order,
                      std::string& status);

 private:
  std::vector<std::pair<match_engine_proto::Order, std::string> >
      order_and_status_;
};

#endif  // ORDER_MANAGER_INCLUDE_ORDER_STORE_MOCK_H_
