/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef CTE_ORDER_GENERATOR_H
#define CTE_ORDER_GENERATOR_H

#include <iostream>
#include <map>

#include "../common/include/influxdb.hpp"
#include "../common/include/json.hpp"

int GnenerateRandomNumber(int range_min, int range_max);
std::map<int, int> GenerateInitialPrice(int object_id_min, int object_id_max,
                                        int price_min, int price_max);
std::map<int, int> GetAllInitialPrice();

class Order {
 public:
  Order(std::map<int, int> &all_initial_prices, int user_id_min,
        int user_id_max, int symbol_min, int symbol_max,
        int amount_max = 10000);
  //  Order( int user_id_min,
  //        int user_id_max, int symbol_min, int symbol_max);
  void CreateOrderInDatabase();
 private:
  int user_id_;
  int symbol_;
  int price_;
  int amount_;
  bool trading_side_;
};

#endif  // CTE_ORDER_GENERATOR_H