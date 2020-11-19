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

  influxdb_cpp::server_info si(
      orders_config["database_host"], orders_config["database_port"],
      orders_config["database_name"], orders_config["database_user"],
      orders_config["database_password"]);
  influxdb_cpp::detail::ts_caller payload;
  for (int i = 0; i < orders_config["order_amount"]; i++) {
    Order order(initial_prices, orders_config["user_id_min"],
                orders_config["user_id_max"],
                orders_config["trading_amount_min"],
                orders_config["trading_amount_max"]);

    payload.meas("orders")
        .field("user_id", order.GetUserId())
        .field("symbol", order.GetSymbol())
        .field("price", order.GetPrice())
        .field("amount", order.GetAmount())
        .field("trading_side", order.GetTradingSide())
        .timestamp(i);

    if ((i + 1) % 10000 == 0 ||
        (orders_config["order_amount"] < 10000 &&
         i == static_cast<int>(orders_config["order_amount"]) - 1)) {
      std::string resp;
      int ret = payload.post_http(si, &resp);
      if (0 != ret || !resp.empty()) {
        std::cout << "write db failed, ret:" << ret << " resp:" << resp
                  << std::endl;
        return ret;
      }
      payload.reset_payload();
      std::cout << "write db success, the round is: " << i + 1 << std::endl;
    }
  }

  return ret;
}
