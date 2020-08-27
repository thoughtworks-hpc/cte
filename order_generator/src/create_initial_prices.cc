/*
 * Copyright (c) 2020 ThoughtWorks Inc.
 */
#include <fstream>

#include "order_generator.h"

int main(int argc, char* argv[]) {
  srand((unsigned)time(nullptr));

  std::ifstream in("../files/create_initial_prices_config.json");
  nlohmann::json initial_prices_config;
  in >> initial_prices_config;

  std::map<int, int> initial_prices = GenerateInitialPrice(
      initial_prices_config["symbol_id_min"],
      initial_prices_config["symbol_id_max"],
      initial_prices_config["price_min"], initial_prices_config["price_max"]);
  int ret = ImportInitialPriceToJsonFile(initial_prices);
  return ret;
}