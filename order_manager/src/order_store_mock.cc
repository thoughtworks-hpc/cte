/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_store_mock.h"
int OrderStoreMock::PersistOrder(const match_engine_proto::Order& order,
                                 std::string status, int concluded_amount) {
  order_and_status_.emplace_back(std::make_pair(order, status));

  return 0;
}
int OrderStoreMock::GetOrderByIndex(int index, match_engine_proto::Order& order,
                                    std::string& status) {
  if (index >= order_and_status_.size()) {
    return 1;
  }

  auto pair = order_and_status_.at(index);
  order = pair.first;
  status = pair.second;

  return 0;
}
