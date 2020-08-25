/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include "./order_generator.h"

int GnenerateRandomNumber(int range_min, int range_max) {
  return rand() % (range_max - range_min + 1) + range_min;
}

std::map<int, int> GenerateInitialPrice(int object_id_min, int object_id_max,
                                        int price_min, int price_max) {
  //  srand((unsigned)time(nullptr));
  std::map<int, int> all_initial_prices;

  std::string resp;
  int ret;
  influxdb_cpp::server_info si("127.0.0.1", 8086, "orders", "", "");

  for (int i = object_id_min; i < object_id_max + 1; i++) {
    int price = GnenerateRandomNumber(price_min * 100, price_max * 100);
    all_initial_prices[i] = price;

    ret = influxdb_cpp::builder()
              .meas("initial_price")
              .tag("key_symbol", std::to_string(i))
              .field("price", price)
              .field("symbol", i)
              .timestamp(0)
              .post_http(si, &resp);

    if (0 == ret && "" == resp) {
      std::cout << "write db success" << std::endl;
    } else {
      std::cout << "write db failed, ret:" << ret << " resp:" << resp
                << std::endl;
    }
  }

  return all_initial_prices;
}

std::map<int, int> GetAllInitialPrice() {
  using json = nlohmann::json;

  std::map<int, int> all_initial_prices;
  std::string resp;
  int ret;
  influxdb_cpp::server_info si("127.0.0.1", 8086, "orders", "", "");

  ret = influxdb_cpp::query(resp, "select * from initial_price", si);
  if (0 == ret) {
    std::cout << "query db success, resp:" << resp << std::endl;
  } else {
    std::cout << "query db failed ret:" << ret << std::endl;
  }

  json j = json::parse(resp);
  for (auto &item : j["results"][0]["series"][0]["values"]) {
    all_initial_prices[item[3]] = item[2];
  }

  return all_initial_prices;
}

Order::Order(std::map<int, int> &all_initial_prices, int user_id_min,
             int user_id_max, int symbol_min, int symbol_max, int amount_max) {
  this->user_id_ = GnenerateRandomNumber(user_id_min, user_id_max);
  this->symbol_ = GnenerateRandomNumber(symbol_min, symbol_max);
  int initial_price = all_initial_prices[this->symbol_];
  this->price_ =
      GnenerateRandomNumber(0.9 * initial_price, 1.1 * initial_price);
  this->amount_ = GnenerateRandomNumber(1, amount_max);
  this->trading_side_ = GnenerateRandomNumber(0, 1);
}

void Order::CreateOrderInDatabase() {
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
}

int main() {
  srand((unsigned)time(nullptr));
//  GenerateInitialPrice(0,10,1,20);

  auto all_initial_prices = GetAllInitialPrice();

  for (int i = 0; i < 20; i++) {
      Order order(all_initial_prices, 1, 10, 1, 10);
      order.CreateOrderInDatabase();
    }

  //  GenerateInitialPrice(0,10,1,20);
  //  GetAllInitialPrice();
  return 0;
}