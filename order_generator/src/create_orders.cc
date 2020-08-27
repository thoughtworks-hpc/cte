/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include "order_generator.h"

int main() {
  srand((unsigned)time(nullptr));
  auto initial_prices = GetAllInitialPrice();

  std::ifstream in("../files/create_orders_config.json");
  nlohmann::json orders_config;
  in >> orders_config;

  for (int i = 0; i < orders_config["order_amount"]; i++) {
    Order order(initial_prices, orders_config["user_id_min"],
                orders_config["user_id_max"], orders_config["symbol_id_min"],
                orders_config["symbol_id_max"],
                orders_config["trading_amount_min"],
                orders_config["trading_amount_max"]);
    order.CreateOrderInDatabase();
  }

  return 0;
}