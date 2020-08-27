/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include "order_generator.h"

int main(int argc, char* argv[]) {
  srand((unsigned)time(nullptr));
  auto initial_prices = GetAllInitialPrice();
  int ret = 0;

  nlohmann::json initial_prices_config;
  nlohmann::json orders_config;
  if (argc == 1) {
    std::ifstream in("create_initial_prices_config.json");
    in >> initial_prices_config;
    std::ifstream in2("create_orders_config.json");
    in2 >> orders_config;
  } else if (argc == 3) {
    std::ifstream in(argv[1]);
    in >> initial_prices_config;
    std::ifstream in2(argv[2]);
    in2 >> orders_config;
  } else {
    ret = 1;
    std::cout << "Error: you must enter two file paths or not." << std::endl;
  }

  for (int i = 0; i < orders_config["order_amount"]; i++) {
    Order order(initial_prices, orders_config["user_id_min"],
                orders_config["user_id_max"],
                initial_prices_config["symbol_id_min"],
                initial_prices_config["symbol_id_max"],
                orders_config["trading_amount_min"],
                orders_config["trading_amount_max"]);
    order.CreateOrderInDatabase();
  }

  return ret;
}