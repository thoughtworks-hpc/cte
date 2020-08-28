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

int CreateDatabaseOrder() {
  std::string resp;
  int ret = 1;
  influxdb_cpp::server_info si("127.0.0.1", 8086, "orders", "", "");
  ret = influxdb_cpp::create_db(resp, "orders", si);
  if (0 == ret) {
    std::cout << "creat db success, resp:" << resp << std::endl;
  } else {
    std::cout << "creat db failed ret:" << ret << std::endl;
  }
  return ret;
}

int ImportInitialPriceToJsonFile(std::map<int, int> initial_prices) {
  nlohmann::json j;
  for (auto& initial_price : initial_prices) {
    j[std::to_string(initial_price.first)] = initial_price.second;
  }
  std::ofstream o("initial_prices.json");
  o << j << std::endl;
  return 0;
}

std::map<int, int> GetAllInitialPrice(std::string file_path) {
  std::map<int, int> all_initial_prices;

  std::ifstream in(file_path);
  nlohmann::json initial_prices;
  in >> initial_prices;

  for (const auto& initial_price : initial_prices.items()) {
    all_initial_prices[stoi(initial_price.key())] = initial_price.value();
  }

  return all_initial_prices;
}

Order::Order(std::map<int, int>& all_initial_prices, int user_id_min,
             int user_id_max, int amount_min, int amount_max) {
  this->user_id_ = GenerateRandomNumber(user_id_min, user_id_max);
  int temp = GenerateRandomNumber(0, (int)all_initial_prices.size() - 1);
  int initial_price;
  int i = 0;
  for (auto& item : all_initial_prices) {
    if (i == temp) {
      this->symbol_ = item.first;
      initial_price = item.second;
      break;
    }
    i++;
  }
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
