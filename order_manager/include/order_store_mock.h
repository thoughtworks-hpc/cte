/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_ORDER_STORE_MOCK_H_
#define ORDER_MANAGER_INCLUDE_ORDER_STORE_MOCK_H_

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "./order_store.h"

class OrderStoreMock : public OrderStore {
 public:
  int PersistOrder(const Order& order, std::string status,
                   int concluded_amount);
  int GetOrderByIndex(int index, Order& order, std::string& status);
  int GetOrderByOrderId(int id, Order& order);

  void GetStatusByOrderId(int id, std::string& status);

  void GetConcludedAmountByOrderId(int id, int32_t& concluded_amount);

 private:
  std::vector<std::pair<Order, std::string> > order_and_status_;
  std::unordered_map<int64_t, Order> order_id_to_order_;
  std::unordered_map<int64_t, std::string> order_id_to_status_;
  std::unordered_map<int64_t, int32_t> order_id_to_concluded_amount_;
};

#endif  // ORDER_MANAGER_INCLUDE_ORDER_STORE_MOCK_H_
