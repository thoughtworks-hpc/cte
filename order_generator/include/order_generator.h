/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#ifndef ORDER_GENERATOR_INCLUDE_ORDER_GENERATOR_H_
#define ORDER_GENERATOR_INCLUDE_ORDER_GENERATOR_H_

#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>

#include "../../common/include/influxdb.hpp"
#include "../../common/include/json.hpp"

int GenerateRandomNumber(const int& range_min, const int& range_max);
int CreateDatabase(const std::string& database_host, const int& database_port,
                   const std::string& database_name,
                   const std::string& database_user,
                   const std::string& database_password);
std::map<int, int> GenerateInitialPrice(const int& symbol_id_min,
                                        const int& symbol_id_max,
                                        const int& price_min,
                                        const int& price_max);
int ImportInitialPriceToJsonFile(const std::map<int, int>& initial_prices);
std::map<int, int> GetAllInitialPrice(const std::string& file_path);

class Order {
 public:
  Order(const std::map<int, int>& all_initial_prices, const int& user_id_min,
        const int& user_id_max, const int& amount_min, const int& amount_max);
  int CreateOrderInDatabase(const std::string& database_host,
                            const int& database_port,
                            const std::string& database_name,
                            const std::string& database_user,
                            const std::string& database_password) const;

 private:
  int user_id_;
  int symbol_;
  int price_;
  int amount_;
  int trading_side_;

 public:
  int GetUserId() const;
  int GetSymbol() const;
  int GetPrice() const;
  int GetAmount() const;
  int GetTradingSide() const;
};

#endif  // ORDER_GENERATOR_INCLUDE_ORDER_GENERATOR_H_
