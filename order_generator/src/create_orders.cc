/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../../common/include/database_interface.hpp"
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

  database_interface::InfluxDB influxdb(si, false);

  for (int i = 0; i < orders_config["order_amount"]; i++) {
    Order order(initial_prices, orders_config["user_id_min"],
                orders_config["user_id_max"],
                orders_config["trading_amount_min"],
                orders_config["trading_amount_max"]);

    std::vector<database_interface::data_pair> tag;
    std::vector<database_interface::data_pair> field;
    field.emplace_back(database_interface::data_pair{
        "user_id", std::to_string(order.GetUserId())});
    field.emplace_back(database_interface::data_pair{
        "symbol", std::to_string(order.GetSymbol())});
    field.emplace_back(database_interface::data_pair{
        "price", std::to_string(order.GetPrice())});
    field.emplace_back(database_interface::data_pair{
        "amount", std::to_string(order.GetAmount())});
    field.emplace_back(database_interface::data_pair{
        "trading_side", std::to_string(order.GetTradingSide())});

    database_interface::entity payload{"orders", tag, field, i};
    influxdb.write(payload);
  }
  influxdb.flush_buffer();

  return ret;
}
