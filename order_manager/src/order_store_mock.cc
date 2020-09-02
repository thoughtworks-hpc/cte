/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#include "../include/order_store_mock.h"

int OrderStoreMock::PersistOrder(const match_engine_proto::Order& order,
                                 std::string status, int concluded_amount) {
  order_and_status_.emplace_back(std::make_pair(order, status));
  order_id_to_order_[order.order_id()] = order;
  order_id_to_status_[order.order_id()] = status;
  order_id_to_concluded_amount_[order.order_id()] = concluded_amount;

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

int OrderStoreMock::GetOrderByOrderId(int id,
                                      match_engine_proto::Order& order) {
  if (order_id_to_order_.find(id) != order_id_to_order_.end()) {
    order = order_id_to_order_[id];
  }

  return 0;
}

void OrderStoreMock::GetStatusByOrderId(int id, std::string& status) {
  if (order_id_to_status_.find(id) != order_id_to_status_.end()) {
    status = order_id_to_status_[id];
  }
}

void OrderStoreMock::GetConcludedAmountByOrderId(int id,
                                                 int32_t& concluded_amount) {
  if (order_id_to_concluded_amount_.find(id) !=
      order_id_to_concluded_amount_.end()) {
    concluded_amount = order_id_to_concluded_amount_[id];
  }
}
