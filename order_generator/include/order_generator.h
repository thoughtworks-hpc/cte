/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef ORDER_GENERATOR_INCLUDE_ORDER_GENERATOR_H_
#define ORDER_GENERATOR_INCLUDE_ORDER_GENERATOR_H_

#include <fstream>
#include <iostream>
#include <map>

#include "../../common/include/influxdb.hpp"
#include "../../common/include/json.hpp"

int GenerateRandomNumber(int range_min, int range_max);
int CreateDatabaseOrder();
std::map<int, int> GenerateInitialPrice(int symbol_id_min, int symbol_id_max,
                                        int price_min, int price_max);
int ImportInitialPriceToJsonFile(std::map<int, int> initial_prices);
std::map<int, int> GetAllInitialPrice();

class Order {
 public:
  Order(std::map<int, int> &all_initial_prices, int user_id_min,
        int user_id_max, int symbol_min, int symbol_max, int amount_min = 100,
        int amount_max = 10000);
  int CreateOrderInDatabase();

 private:
  int user_id_;
  int symbol_;
  int price_;
  int amount_;
  bool trading_side_;

 public:
  int GetUserId() const;
  int GetSymbol() const;
  int GetPrice() const;
  int GetAmount() const;
};

#endif  // ORDER_GENERATOR_INCLUDE_ORDER_GENERATOR_H_
