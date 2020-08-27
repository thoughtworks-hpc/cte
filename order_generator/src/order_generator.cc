/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include "../include/order_generator.h"

int GenerateRandomNumber(int range_min, int range_max) {
  return rand() % (range_max - range_min + 1) + range_min;
}

std::map<int, int> GenerateInitialPrice(int symbol_id_min, int symbol_id_max,
                                        int price_min, int price_max) {
  std::map<int, int> all_initial_prices;

  for (int i = symbol_id_min; i < symbol_id_max + 1; i++) {
    int price = GenerateRandomNumber(price_min * 100, price_max * 100);
    all_initial_prices[i] = price;
  }

  return all_initial_prices;
}

int ImportInitialPriceToJsonFile(std::map<int, int> initial_prices) {
  nlohmann::json j;
  for (auto& initial_price : initial_prices) {
    j[initial_price.first] = initial_price.second;
  }
  std::ofstream o("../files/initial_prices.json");
  o << j << std::endl;
  return 0;
}

std::map<int, int> GetAllInitialPrice() {
  std::map<int, int> all_initial_prices;

  std::ifstream in("../files/initial_prices.json");
  nlohmann::json initial_prices;
  in >> initial_prices;

  for (int i = 0; i < initial_prices.size(); i++) {
    if (!initial_prices[i].empty()) {
      all_initial_prices[i] = initial_prices[i];
    }
  }

  for (auto& item : all_initial_prices) {
    std::cout << item.first << " " << item.second << std::endl;
  }

  return all_initial_prices;
}

Order::Order(std::map<int, int>& all_initial_prices, int user_id_min,
             int user_id_max, int symbol_min, int symbol_max, int amount_min,
             int amount_max) {
  this->user_id_ = GenerateRandomNumber(user_id_min, user_id_max);
  this->symbol_ = GenerateRandomNumber(symbol_min, symbol_max);
  int initial_price = all_initial_prices[this->symbol_];
  this->price_ = GenerateRandomNumber(0.9 * initial_price, 1.1 * initial_price);
  this->amount_ = GenerateRandomNumber(amount_min, amount_max);
  this->trading_side_ = GenerateRandomNumber(0, 1);
}

int Order::CreateOrderInDatabase() {
  std::string resp;
  int ret;
  influxdb_cpp::server_info si("127.0.0.1", 8086, "orders", "", "");

  ret = influxdb_cpp::builder()
            .meas("orders")
            .field("user_id", (int32_t)this->user_id_)
            .field("symbol", (int32_t)this->symbol_)
            .field("price", (int32_t)this->price_)
            .field("amount", (int32_t)this->amount_)
            .field("trading_side", (int32_t)this->trading_side_)
            .post_http(si, &resp);

  if (0 == ret && "" == resp) {
    std::cout << "write db success" << std::endl;
  } else {
    std::cout << "write db failed, ret:" << ret << " resp:" << resp
              << std::endl;
  }

  return ret;
}
int Order::GetUserId() const { return user_id_; }
int Order::GetSymbol() const { return symbol_; }
int Order::GetPrice() const { return price_; }
int Order::GetAmount() const { return amount_; }