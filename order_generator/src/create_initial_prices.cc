/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */

#include "../include/order_generator.h"

int main(int argc, char* argv[]) {
  srand((unsigned)time(nullptr));

  int ret;
  nlohmann::json initial_prices_config;

  ret = CreateDatabaseOrder();
  if (ret == 1) {
    std::cout << "Error: create db failed" << std::endl;
    return ret;
  }

  if (argc == 1) {
    std::ifstream input("create_initial_prices_config.json");
    input >> initial_prices_config;
  } else if (argc == 2) {
    std::ifstream in(argv[1]);
    in >> initial_prices_config;
  } else {
    ret = 1;
    std::cout << "Error: you must enter one file path or not." << std::endl;
    return ret;
  }

  std::map<int, int> initial_prices = GenerateInitialPrice(
      initial_prices_config["symbol_id_min"],
      initial_prices_config["symbol_id_max"],
      initial_prices_config["price_min"], initial_prices_config["price_max"]);
  ret = ImportInitialPriceToJsonFile(initial_prices);
  return ret;
}
