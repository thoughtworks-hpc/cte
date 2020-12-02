/*
 * Copyright (c) 2019-2020 ThoughtWorks Inc.
 */

#ifndef ORDER_MANAGER_INCLUDE_ORDER_H_
#define ORDER_MANAGER_INCLUDE_ORDER_H_

#include <cstdint>
struct Order {
 public:
  int64_t order_id{};
  int32_t symbol_id{};
  int32_t user_id{};
  int32_t price{};
  int32_t amount{};
  int8_t trading_side{};
  int64_t submit_time{};
};

enum OrderTradingSide { Buy = 1, Sell = 2, Unknown = 3 };

#endif  // ORDER_MANAGER_INCLUDE_ORDER_H_
