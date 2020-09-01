/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include "../include/order_generator.h"

int main(int argc, char* argv[]) {
  int ret = 0;
  std::string initial_price_file_path;

  nlohmann::json orders_config;
  if (argc == 1) {
    initial_price_file_path = "initial_prices.json";
    std::ifstream input("create_orders_config.json");
    input >> orders_config;
  } else if (argc == 3) {
    initial_price_file_path = argv[1];
    std::ifstream in2(argv[2]);
    in2 >> orders_config;
  } else {
    ret = 1;
    std::cout << "Error: you must enter two file paths or not." << std::endl;
  }

  ret = CreateDatabase(
      orders_config["database_host"], orders_config["database_port"],
      orders_config["database_name"], orders_config["database_user"],
      orders_config["database_password"]);
  if (ret == 1) {
    std::cout << "Error: create db failed" << std::endl;
    return ret;
  }

  auto initial_prices = GetAllInitialPrice(initial_price_file_path);

  for (int i = 0; i < orders_config["order_amount"]; i++) {
    Order order(initial_prices, orders_config["user_id_min"],
                orders_config["user_id_max"],
                orders_config["trading_amount_min"],
                orders_config["trading_amount_max"]);
    order.CreateOrderInDatabase(
        orders_config["database_host"], orders_config["database_port"],
        orders_config["database_name"], orders_config["database_user"],
        orders_config["database_password"]);
    if (i % 1000 == 0) {
      std::cout << "write db success, the round is: " << i << std::endl;
    }
  }

  return ret;
}
